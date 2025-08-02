/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "soc/soc_caps.h"

#if SOC_MIPI_DSI_SUPPORTED
#include "esp_check.h"
#include "esp_log.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_vendor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "lcd_h070b13.h"

#define h070b13_PAD_CONTROL                  (0xB7)
#define h070b13_DSI_2_LANE                   (0x03)
#define h070b13_DSI_3_4_LANE                 (0x02)

#define h070b13_CMD_GS_BIT       (1 << 0)
#define h070b13_CMD_SS_BIT       (1 << 1)

typedef struct {
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
    const h070b13_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t lane_num;
    struct {
        unsigned int reset_level : 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} h070b13_panel_t;

static const char *TAG = "h070b13";

static esp_err_t panel_h070b13_del(esp_lcd_panel_t *panel);
static esp_err_t panel_h070b13_init(esp_lcd_panel_t *panel);
static esp_err_t panel_h070b13_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_h070b13_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_h070b13_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_h070b13_disp_on_off(esp_lcd_panel_t *panel, bool on_off);
static esp_err_t panel_h070b13_sleep(esp_lcd_panel_t *panel, bool sleep);

esp_err_t esp_lcd_new_panel_h070b13(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel) {
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    h070b13_vendor_config_t *vendor_config = (h070b13_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)calloc(1, sizeof(h070b13_panel_t));
    ESP_RETURN_ON_FALSE(h070b13, ESP_ERR_NO_MEM, TAG, "no mem for h070b13 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->rgb_ele_order) {
        case LCD_RGB_ELEMENT_ORDER_RGB:
            h070b13->madctl_val = 0;
            break;
        case LCD_RGB_ELEMENT_ORDER_BGR:
            h070b13->madctl_val |= LCD_CMD_BGR_BIT;
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
            break;
    }

    switch (panel_dev_config->bits_per_pixel) {
        case 16: // RGB565
            h070b13->colmod_val = 0x55;
            break;
        case 18: // RGB666
            h070b13->colmod_val = 0x66;
            break;
        case 24: // RGB888
            h070b13->colmod_val = 0x77;
            break;
        default:
            ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
            break;
    }

    h070b13->io = io;
    h070b13->init_cmds = vendor_config->init_cmds;
    h070b13->init_cmds_size = vendor_config->init_cmds_size;
    h070b13->reset_gpio_num = panel_dev_config->reset_gpio_num;
    h070b13->flags.reset_level = panel_dev_config->flags.reset_active_high;

    // Create MIPI DPI panel
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
        "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    // Save the original functions of MIPI DPI panel
    h070b13->del = (*ret_panel)->del;
    h070b13->init = (*ret_panel)->init;
    // Overwrite the functions of MIPI DPI panel
    (*ret_panel)->del = panel_h070b13_del;
    (*ret_panel)->init = panel_h070b13_init;
    (*ret_panel)->reset = panel_h070b13_reset;
    (*ret_panel)->mirror = panel_h070b13_mirror;
    (*ret_panel)->invert_color = panel_h070b13_invert_color;
    (*ret_panel)->disp_on_off = panel_h070b13_disp_on_off;
    (*ret_panel)->disp_sleep = panel_h070b13_sleep;
    (*ret_panel)->user_data = h070b13;
    ESP_LOGD(TAG, "new h070b13 panel @%p", h070b13);

    return ESP_OK;

err:
    if (h070b13) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(h070b13);
    }
    return ret;
}

static const h070b13_lcd_init_cmd_t vendor_specific_init_default[] = {
    // {cmd, { data }, data_size, delay_ms}
    /**** CMD_Page 3 ****/
    {0xFF, (uint8_t[]) { 0x98, 0x81, 0x03 }, 3, 0},
    {0x01, (uint8_t[]) { 0x00 }, 1, 0},
    {0x02, (uint8_t[]) { 0x00 }, 1, 0},
    {0x03, (uint8_t[]) { 0x73 }, 1, 0},
    {0x04, (uint8_t[]) { 0x13 }, 1, 0},
    {0x05, (uint8_t[]) { 0x00 }, 1, 0},
    {0x06, (uint8_t[]) { 0x0A }, 1, 0},
    {0x07, (uint8_t[]) { 0x05 }, 1, 0},
    {0x08, (uint8_t[]) { 0x00 }, 1, 0},
    {0x09, (uint8_t[]) { 0x28 }, 1, 0},
    {0x0a, (uint8_t[]) { 0x00 }, 1, 0},
    {0x0b, (uint8_t[]) { 0x00 }, 1, 0},
    {0x0c, (uint8_t[]) { 0x00 }, 1, 0},
    {0x0d, (uint8_t[]) { 0x28 }, 1, 0},
    {0x0e, (uint8_t[]) { 0x00 }, 1, 0},
    {0x0f, (uint8_t[]) { 0x28 }, 1, 0},
    {0x10, (uint8_t[]) { 0x28 }, 1, 0},
    {0x11, (uint8_t[]) { 0x00 }, 1, 0},
    {0x12, (uint8_t[]) { 0x00 }, 1, 0},
    {0x13, (uint8_t[]) { 0x00 }, 1, 0},
    {0x14, (uint8_t[]) { 0x00 }, 1, 0},
    {0x15, (uint8_t[]) { 0x00 }, 1, 0},
    {0x16, (uint8_t[]) { 0x00 }, 1, 0},
    {0x17, (uint8_t[]) { 0x00 }, 1, 0},
    {0x18, (uint8_t[]) { 0x00 }, 1, 0},
    {0x19, (uint8_t[]) { 0x00 }, 1, 0},
    {0x1a, (uint8_t[]) { 0x00 }, 1, 0},
    {0x1b, (uint8_t[]) { 0x00 }, 1, 0},
    {0x1c, (uint8_t[]) { 0x00 }, 1, 0},
    {0x1d, (uint8_t[]) { 0x00 }, 1, 0},
    {0x1e, (uint8_t[]) { 0x40 }, 1, 0},
    {0x1f, (uint8_t[]) { 0x80 }, 1, 0},
    {0x20, (uint8_t[]) { 0x06 }, 1, 0},
    {0x21, (uint8_t[]) { 0x01 }, 1, 0},
    {0x22, (uint8_t[]) { 0x00 }, 1, 0},
    {0x23, (uint8_t[]) { 0x00 }, 1, 0},
    {0x24, (uint8_t[]) { 0x00 }, 1, 0},
    {0x25, (uint8_t[]) { 0x00 }, 1, 0},
    {0x26, (uint8_t[]) { 0x00 }, 1, 0},
    {0x27, (uint8_t[]) { 0x00 }, 1, 0},
    {0x28, (uint8_t[]) { 0x33 }, 1, 0},
    {0x29, (uint8_t[]) { 0x33 }, 1, 0},
    {0x2a, (uint8_t[]) { 0x00 }, 1, 0},
    {0x2b, (uint8_t[]) { 0x00 }, 1, 0},
    {0x2c, (uint8_t[]) { 0x04 }, 1, 0},
    {0x2d, (uint8_t[]) { 0x04 }, 1, 0},
    {0x2e, (uint8_t[]) { 0x05 }, 1, 0},
    {0x2f, (uint8_t[]) { 0x05 }, 1, 0},
    {0x30, (uint8_t[]) { 0x00 }, 1, 0},
    {0x31, (uint8_t[]) { 0x00 }, 1, 0},
    {0x32, (uint8_t[]) { 0x31 }, 1, 0},
    {0x33, (uint8_t[]) { 0x00 }, 1, 0},
    {0x34, (uint8_t[]) { 0x00 }, 1, 0},
    {0x35, (uint8_t[]) { 0x0A }, 1, 0},
    {0x36, (uint8_t[]) { 0x00 }, 1, 0},
    {0x37, (uint8_t[]) { 0x08 }, 1, 0},
    {0x38, (uint8_t[]) { 0x00 }, 1, 0},
    {0x39, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3a, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3b, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3c, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3d, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3e, (uint8_t[]) { 0x00 }, 1, 0},
    {0x3f, (uint8_t[]) { 0x00 }, 1, 0},
    {0x40, (uint8_t[]) { 0x00 }, 1, 0},
    {0x41, (uint8_t[]) { 0x00 }, 1, 0},
    {0x42, (uint8_t[]) { 0x00 }, 1, 0},
    {0x43, (uint8_t[]) { 0x08 }, 1, 0},
    {0x44, (uint8_t[]) { 0x00 }, 1, 0},
    {0x50, (uint8_t[]) { 0x01 }, 1, 0},
    {0x51, (uint8_t[]) { 0x23 }, 1, 0},
    {0x52, (uint8_t[]) { 0x44 }, 1, 0},
    {0x53, (uint8_t[]) { 0x67 }, 1, 0},
    {0x54, (uint8_t[]) { 0x89 }, 1, 0},
    {0x55, (uint8_t[]) { 0xab }, 1, 0},
    {0x56, (uint8_t[]) { 0x01 }, 1, 0},
    {0x57, (uint8_t[]) { 0x23 }, 1, 0},
    {0x58, (uint8_t[]) { 0x45 }, 1, 0},
    {0x59, (uint8_t[]) { 0x67 }, 1, 0},
    {0x5a, (uint8_t[]) { 0x89 }, 1, 0},
    {0x5b, (uint8_t[]) { 0xab }, 1, 0},
    {0x5c, (uint8_t[]) { 0xcd }, 1, 0},
    {0x5d, (uint8_t[]) { 0xef }, 1, 0},
    {0x5e, (uint8_t[]) { 0x11 }, 1, 0},
    {0x5f, (uint8_t[]) { 0x02 }, 1, 0},
    {0x60, (uint8_t[]) { 0x08 }, 1, 0},
    {0x61, (uint8_t[]) { 0x0E }, 1, 0},
    {0x62, (uint8_t[]) { 0x0F }, 1, 0},
    {0x63, (uint8_t[]) { 0x0C }, 1, 0},
    {0x64, (uint8_t[]) { 0x0D }, 1, 0},
    {0x65, (uint8_t[]) { 0x17 }, 1, 0},
    {0x66, (uint8_t[]) { 0x01 }, 1, 0},
    {0x67, (uint8_t[]) { 0x01 }, 1, 0},
    {0x68, (uint8_t[]) { 0x02 }, 1, 0},
    {0x69, (uint8_t[]) { 0x02 }, 1, 0},
    {0x6a, (uint8_t[]) { 0x00 }, 1, 0},
    {0x6b, (uint8_t[]) { 0x00 }, 1, 0},
    {0x6c, (uint8_t[]) { 0x02 }, 1, 0},
    {0x6d, (uint8_t[]) { 0x02 }, 1, 0},
    {0x6e, (uint8_t[]) { 0x16 }, 1, 0},
    {0x6f, (uint8_t[]) { 0x16 }, 1, 0},
    {0x70, (uint8_t[]) { 0x06 }, 1, 0},
    {0x71, (uint8_t[]) { 0x06 }, 1, 0},
    {0x72, (uint8_t[]) { 0x07 }, 1, 0},
    {0x73, (uint8_t[]) { 0x07 }, 1, 0},
    {0x74, (uint8_t[]) { 0x02 }, 1, 0},
    {0x75, (uint8_t[]) { 0x02 }, 1, 0},
    {0x76, (uint8_t[]) { 0x08 }, 1, 0},
    {0x77, (uint8_t[]) { 0x0E }, 1, 0},
    {0x78, (uint8_t[]) { 0x0F }, 1, 0},
    {0x79, (uint8_t[]) { 0x0C }, 1, 0},
    {0x7a, (uint8_t[]) { 0x0D }, 1, 0},
    {0x7b, (uint8_t[]) { 0x17 }, 1, 0},
    {0x7c, (uint8_t[]) { 0x01 }, 1, 0},
    {0x7d, (uint8_t[]) { 0x01 }, 1, 0},
    {0x7e, (uint8_t[]) { 0x02 }, 1, 0},
    {0x7f, (uint8_t[]) { 0x02 }, 1, 0},
    {0x80, (uint8_t[]) { 0x00 }, 1, 0},
    {0x81, (uint8_t[]) { 0x00 }, 1, 0},
    {0x82, (uint8_t[]) { 0x02 }, 1, 0},
    {0x83, (uint8_t[]) { 0x02 }, 1, 0},
    {0x84, (uint8_t[]) { 0x16 }, 1, 0},
    {0x85, (uint8_t[]) { 0x16 }, 1, 0},
    {0x86, (uint8_t[]) { 0x06 }, 1, 0},
    {0x87, (uint8_t[]) { 0x06 }, 1, 0},
    {0x88, (uint8_t[]) { 0x07 }, 1, 0},
    {0x89, (uint8_t[]) { 0x07 }, 1, 0},
    {0x8A, (uint8_t[]) { 0x02 }, 1, 0},
    //CMD PAGE 4
    {0xFF, (uint8_t[]) { 0x98, 0x81, 0x04 }, 3, 0},
    {0x6E, (uint8_t[]) { 0x1A }, 1, 0 },
    {0x6F, (uint8_t[]) { 0x37 }, 1, 0},
    {0x3A, (uint8_t[]) { 0xA4 }, 1, 0},
    {0x8D, (uint8_t[]) { 0x1F }, 1, 0},
    {0x87, (uint8_t[]) { 0xBA }, 1, 0},
    {0xB2, (uint8_t[]) { 0xD1 }, 1, 0},
    {0x88, (uint8_t[]) { 0x0B }, 1, 0},
    {0x38, (uint8_t[]) { 0x01 }, 1, 0},
    {0x39, (uint8_t[]) { 0x00 }, 1, 0},
    {0xB5, (uint8_t[]) { 0x02 }, 1, 0},
    {0x31, (uint8_t[]) { 0x25 }, 1, 0},
    {0x3B, (uint8_t[]) { 0x98 }, 1, 0},
    //CMD PAGE 1
    {0xFF, (uint8_t[]) { 0x98, 0x81, 0x01 }, 3, 0},
    {0x22, (uint8_t[]) { 0x0A }, 1, 0},
    {0x31, (uint8_t[]) { 0x00 }, 1, 0},
    {0x53, (uint8_t[]) { 0x3D }, 1, 0},
    {0x55, (uint8_t[]) { 0x3D }, 1, 0},
    {0x50, (uint8_t[]) { 0xA0 }, 1, 0},
    {0x51, (uint8_t[]) { 0x9C }, 1, 0},
    {0x60, (uint8_t[]) { 0x06 }, 1, 0},
    {0x62, (uint8_t[]) { 0x20 }, 1, 0},
    {0xA0, (uint8_t[]) { 0x00 }, 1, 0},
    {0xA1, (uint8_t[]) { 0x21 }, 1, 0},
    {0xA2, (uint8_t[]) { 0x35 }, 1, 0},
    {0xA3, (uint8_t[]) { 0x19 }, 1, 0},
    {0xA4, (uint8_t[]) { 0x1E }, 1, 0},
    {0xA5, (uint8_t[]) { 0x33 }, 1, 0},
    {0xA6, (uint8_t[]) { 0x27 }, 1, 0},
    {0xA7, (uint8_t[]) { 0x26 }, 1, 0},
    {0xA8, (uint8_t[]) { 0xAF }, 1, 0},
    {0xA9, (uint8_t[]) { 0x1B }, 1, 0},
    {0xAA, (uint8_t[]) { 0x27 }, 1, 0},
    {0xAB, (uint8_t[]) { 0x8D }, 1, 0},
    {0xAC, (uint8_t[]) { 0x1A }, 1, 0},
    {0xAD, (uint8_t[]) { 0x1B }, 1, 0},
    {0xAE, (uint8_t[]) { 0x50 }, 1, 0},
    {0xAF, (uint8_t[]) { 0x26 }, 1, 0},
    {0xB0, (uint8_t[]) { 0x2B }, 1, 0},
    {0xB1, (uint8_t[]) { 0x54 }, 1, 0},
    {0xB2, (uint8_t[]) { 0x5E }, 1, 0},
    {0xB3, (uint8_t[]) { 0x23 }, 1, 0},

    // {0xB6, (uint8_t[]) { 0x90 }, 1, 0},
    {0xB7, (uint8_t[]) { 0x03 }, 1, 0},   //select 2 lane

    {0xC0, (uint8_t[]) { 0x00 }, 1, 0},
    {0xC1, (uint8_t[]) { 0x21 }, 1, 0},
    {0xC2, (uint8_t[]) { 0x35 }, 1, 0},
    {0xC3, (uint8_t[]) { 0x19 }, 1, 0},
    {0xC4, (uint8_t[]) { 0x1E }, 1, 0},
    {0xC5, (uint8_t[]) { 0x33 }, 1, 0},
    {0xC6, (uint8_t[]) { 0x27 }, 1, 0},
    {0xC7, (uint8_t[]) { 0x26 }, 1, 0},
    {0xC8, (uint8_t[]) { 0xAF }, 1, 0},
    {0xC9, (uint8_t[]) { 0x1B }, 1, 0},
    {0xCA, (uint8_t[]) { 0x27 }, 1, 0},
    {0xCB, (uint8_t[]) { 0x8D }, 1, 0},
    {0xCC, (uint8_t[]) { 0x1A }, 1, 0},
    {0xCD, (uint8_t[]) { 0x1B }, 1, 0},
    {0xCE, (uint8_t[]) { 0x50 }, 1, 0},
    {0xCF, (uint8_t[]) { 0x26 }, 1, 0},
    {0xD0, (uint8_t[]) { 0x2B }, 1, 0},
    {0xD1, (uint8_t[]) { 0x54 }, 1, 0},
    {0xD2, (uint8_t[]) { 0x5E }, 1, 0},
    {0xD3, (uint8_t[]) { 0x23 }, 1, 0},
    //CMD PAGE 0
    {0xFF, (uint8_t[]) { 0x98, 0x81, 0x00 }, 3, 0},
    {0x35, (uint8_t[]) { 0x00 }, 1, 0},

    {0x11, (uint8_t[]) { 0x00 }, 0, 120},
    {0x29, (uint8_t[]) { 0x00 }, 0, 120},

    //============ Gamma END===========
};

static esp_err_t panel_h070b13_del(esp_lcd_panel_t *panel) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;

    if (h070b13->reset_gpio_num >= 0) {
        gpio_reset_pin(h070b13->reset_gpio_num);
    }
    // Delete MIPI DPI panel
    h070b13->del(panel);
    ESP_LOGD(TAG, "del h070b13 panel @%p", h070b13);
    free(h070b13);

    return ESP_OK;
}

static esp_err_t panel_h070b13_init(esp_lcd_panel_t *panel) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;
    const h070b13_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    bool is_command0_enable = false;
    bool is_cmd_overwritten = false;

    // The ID register is on the CMD_Page 1
    uint8_t ID1, ID2, ID3;
    esp_lcd_panel_io_tx_param(io, 0xFF, (uint8_t[]) { 0x98, 0x81, 0x01 }, 3);
    esp_lcd_panel_io_rx_param(io, 0x00, &ID1, 1);
    esp_lcd_panel_io_rx_param(io, 0x01, &ID2, 1);
    esp_lcd_panel_io_rx_param(io, 0x02, &ID3, 1);
    ESP_LOGI(TAG, "LCD ID: %02X %02X %02X", ID1, ID2, ID3);

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    if (h070b13->init_cmds) {
        init_cmds = h070b13->init_cmds;
        init_cmds_size = h070b13->init_cmds_size;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(h070b13_lcd_init_cmd_t);
    }

    for (int i = 0; i < init_cmds_size; i++) {
        // Check if the command has been used or conflicts with the internal
        if (is_command0_enable && init_cmds[i].data_bytes > 0) {
            switch (init_cmds[i].cmd) {
                case LCD_CMD_MADCTL:
                    is_cmd_overwritten = true;
                    h070b13->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                    break;
                case LCD_CMD_COLMOD:
                    is_cmd_overwritten = true;
                    h070b13->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
                    break;
                default:
                    is_cmd_overwritten = false;
                    break;
            }

            if (is_cmd_overwritten) {
                is_cmd_overwritten = false;
                ESP_LOGW(TAG, "The %02Xh command has been used and will be overwritten by external initialization sequence",
                    init_cmds[i].cmd);
            }
        }

        // Send command
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
    }
    ESP_LOGI(TAG, "send init commands success");

    ESP_RETURN_ON_ERROR(h070b13->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_h070b13_reset(esp_lcd_panel_t *panel) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;

    // Perform hardware reset
    if (h070b13->reset_gpio_num >= 0) {
        gpio_set_level(h070b13->reset_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(h070b13->reset_gpio_num, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(h070b13->reset_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else if (io) { // Perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

static esp_err_t panel_h070b13_invert_color(esp_lcd_panel_t *panel, bool invert_color_data) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;
    uint8_t command = 0;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    if (invert_color_data) {
        command = LCD_CMD_INVON;
    } else {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");

    return ESP_OK;
}

static esp_err_t panel_h070b13_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;
    uint8_t madctl_val = h070b13->madctl_val;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    // Control mirror through LCD command
    if (mirror_x) {
        madctl_val |= h070b13_CMD_GS_BIT;
    } else {
        madctl_val &= ~h070b13_CMD_GS_BIT;
    }
    if (mirror_y) {
        madctl_val |= h070b13_CMD_SS_BIT;
    } else {
        madctl_val &= ~h070b13_CMD_SS_BIT;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        madctl_val
    }, 1), TAG, "send command failed");
    h070b13->madctl_val = madctl_val;

    return ESP_OK;
}

static esp_err_t panel_h070b13_disp_on_off(esp_lcd_panel_t *panel, bool on_off) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;
    int command = 0;

    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_h070b13_sleep(esp_lcd_panel_t *panel, bool sleep) {
    h070b13_panel_t *h070b13 = (h070b13_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h070b13->io;
    int command = 0;

    if (sleep) {
        command = LCD_CMD_SLPIN;
    } else {
        command = LCD_CMD_SLPOUT;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    vTaskDelay(pdMS_TO_TICKS(100));

    return ESP_OK;
}
#endif  // SOC_MIPI_DSI_SUPPORTED
