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
#include "lcd_h013a08.h"

#define h013a08_PAD_CONTROL                  (0xB7)
#define h013a08_DSI_2_LANE                   (0x03)
#define h013a08_DSI_3_4_LANE                 (0x02)

#define h013a08_CMD_GS_BIT       (1 << 0)
#define h013a08_CMD_SS_BIT       (1 << 1)

typedef struct {
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // save current value of LCD_CMD_MADCTL register
    uint8_t colmod_val; // save surrent value of LCD_CMD_COLMOD register
    const h013a08_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t lane_num;
    struct {
        unsigned int reset_level : 1;
    } flags;
    // To save the original functions of MIPI DPI panel
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} h013a08_panel_t;

static const char *TAG = "h013a08";

static esp_err_t panel_h013a08_del(esp_lcd_panel_t *panel);
static esp_err_t panel_h013a08_init(esp_lcd_panel_t *panel);
static esp_err_t panel_h013a08_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_h013a08_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_h013a08_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_h013a08_disp_on_off(esp_lcd_panel_t *panel, bool on_off);
static esp_err_t panel_h013a08_sleep(esp_lcd_panel_t *panel, bool sleep);

esp_err_t esp_lcd_new_panel_h013a08(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
    esp_lcd_panel_handle_t *ret_panel) {
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    h013a08_vendor_config_t *vendor_config = (h013a08_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)calloc(1, sizeof(h013a08_panel_t));
    ESP_RETURN_ON_FALSE(h013a08, ESP_ERR_NO_MEM, TAG, "no mem for h013a08 panel");

    if (panel_dev_config->reset_gpio_num >= 0) {
        gpio_config_t io_conf = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    switch (panel_dev_config->rgb_ele_order) {
    case LCD_RGB_ELEMENT_ORDER_RGB:
        h013a08->madctl_val = 0;
        break;
    case LCD_RGB_ELEMENT_ORDER_BGR:
        h013a08->madctl_val |= LCD_CMD_BGR_BIT;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported color space");
        break;
    }

    switch (panel_dev_config->bits_per_pixel) {
    case 16: // RGB565
        h013a08->colmod_val = 0x55;
        break;
    case 18: // RGB666
        h013a08->colmod_val = 0x66;
        break;
    case 24: // RGB888
        h013a08->colmod_val = 0x77;
        break;
    default:
        ESP_GOTO_ON_FALSE(false, ESP_ERR_NOT_SUPPORTED, err, TAG, "unsupported pixel width");
        break;
    }

    h013a08->io = io;
    h013a08->init_cmds = vendor_config->init_cmds;
    h013a08->init_cmds_size = vendor_config->init_cmds_size;
    h013a08->reset_gpio_num = panel_dev_config->reset_gpio_num;
    h013a08->flags.reset_level = panel_dev_config->flags.reset_active_high;

    // Create MIPI DPI panel
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
        "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    // Save the original functions of MIPI DPI panel
    h013a08->del = (*ret_panel)->del;
    h013a08->init = (*ret_panel)->init;
    // Overwrite the functions of MIPI DPI panel
    (*ret_panel)->del = panel_h013a08_del;
    (*ret_panel)->init = panel_h013a08_init;
    (*ret_panel)->reset = panel_h013a08_reset;
    (*ret_panel)->mirror = panel_h013a08_mirror;
    (*ret_panel)->invert_color = panel_h013a08_invert_color;
    (*ret_panel)->disp_on_off = panel_h013a08_disp_on_off;
    (*ret_panel)->disp_sleep = panel_h013a08_sleep;
    (*ret_panel)->user_data = h013a08;
    ESP_LOGD(TAG, "new h013a08 panel @%p", h013a08);

    return ESP_OK;

err:
    if (h013a08) {
        if (panel_dev_config->reset_gpio_num >= 0) {
            gpio_reset_pin(panel_dev_config->reset_gpio_num);
        }
        free(h013a08);
    }
    return ret;
}

static const h013a08_lcd_init_cmd_t vendor_specific_init_default[] = {
    // {cmd, { data }, data_size, delay_ms}
    /**** CMD_Page 3 ****/
    {0xF0, (uint8_t[]) { 0x00, 0x28 }, 2, 0},
    {0xF2, (uint8_t[]) { 0x00, 0x28 }, 2, 0},
    {0x73, (uint8_t[]) { 0x00, 0xF0 }, 2, 0},
    {0x7C, (uint8_t[]) { 0x00, 0xD1 }, 2, 0},
    {0x83, (uint8_t[]) { 0x00, 0xE0 }, 2, 0},
    {0x84, (uint8_t[]) { 0x00, 0x61 }, 2, 0},
    {0xF2, (uint8_t[]) { 0x00, 0x82 }, 2, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xF1, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xB0, (uint8_t[]) { 0x00, 0x50 }, 2, 0},
    {0xB1, (uint8_t[]) { 0x00, 0x23 }, 2, 0},
    {0xB2, (uint8_t[]) { 0x00, 0x36 }, 2, 0},
    {0xB3, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xB4, (uint8_t[]) { 0x00, 0x06 }, 2, 0},
    {0xB5, (uint8_t[]) { 0x00, 0x24 }, 2, 0},
    {0xB6, (uint8_t[]) { 0x00, 0xA5 }, 2, 0},
    {0xB7, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xB8, (uint8_t[]) { 0x00, 0x8C }, 2, 0},
    {0xB9, (uint8_t[]) { 0x00, 0x15 }, 2, 0},
    {0xBA, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBB, (uint8_t[]) { 0x00, 0x08 }, 2, 0},
    {0xBC, (uint8_t[]) { 0x00, 0x08 }, 2, 0},
    {0xBD, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBE, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBF, (uint8_t[]) { 0x00, 0x07 }, 2, 0},
    {0xC0, (uint8_t[]) { 0x00, 0x80 }, 2, 0},
    {0xC1, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xC2, (uint8_t[]) { 0x00, 0x37 }, 2, 0},
    {0xC3, (uint8_t[]) { 0x00, 0x80 }, 2, 0},
    {0xC4, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xC5, (uint8_t[]) { 0x00, 0x37 }, 2, 0},
    {0xC6, (uint8_t[]) { 0x00, 0xA9 }, 2, 0},
    {0xC7, (uint8_t[]) { 0x00, 0x41 }, 2, 0},
    {0xC8, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xC9, (uint8_t[]) { 0x00, 0xA9 }, 2, 0},
    {0xCA, (uint8_t[]) { 0x00, 0x41 }, 2, 0},
    {0xCB, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xCC, (uint8_t[]) { 0x00, 0x7F }, 2, 0},
    {0xCD, (uint8_t[]) { 0x00, 0x7F }, 2, 0},
    {0xCE, (uint8_t[]) { 0x00, 0xFF }, 2, 0},
    {0xD0, (uint8_t[]) { 0x00, 0x91 }, 2, 0},
    {0xD1, (uint8_t[]) { 0x00, 0x68 }, 2, 0},
    {0xD2, (uint8_t[]) { 0x00, 0x68 }, 2, 0},
    {0xF5, (uint8_t[]) { 0x00, 0x00, 0xA5 }, 3, 0},
    {0xF1, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0xe0, (uint8_t[]) { 0x00, 0xf0, 0x00, 0x0B, 0x00, 0x12, 0x00, 0x0B, 0x00, 0x0A, 0x00, 0x06, 0x00, 0x39, 0x00, 0x43, 0x00, 0x4F, 0x00, 0x07, 0x00, 0x14, 0x00, 0x14, 0x00, 0x2f, 0x00, 0x34 }, 28, 0},
    {0xe1, (uint8_t[]) { 0x00, 0xf0, 0x00, 0x0B, 0x00, 0x11, 0x00, 0x0A, 0x00, 0x09, 0x00, 0x05, 0x00, 0x32, 0x00, 0x33, 0x00, 0x48, 0x00, 0x07, 0x00, 0x13, 0x00, 0x13, 0x00, 0x2C, 0x00, 0x33 }, 28, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xF3, (uint8_t[]) { 0x00, 0x10 }, 2, 0},
    {0xE0, (uint8_t[]) { 0x00, 0x0A }, 2, 0},
    {0xE1, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xE2, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xE3, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xE4, (uint8_t[]) { 0x00, 0xE0 }, 2, 0},
    {0xE5, (uint8_t[]) { 0x00, 0x06 }, 2, 0},
    {0xE6, (uint8_t[]) { 0x00, 0x21 }, 2, 0},
    {0xE7, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xE8, (uint8_t[]) { 0x00, 0x05 }, 2, 0},
    {0xE9, (uint8_t[]) { 0x00, 0xF2 }, 2, 0},
    {0xEA, (uint8_t[]) { 0x00, 0xDF }, 2, 0},
    {0xEB, (uint8_t[]) { 0x00, 0x80 }, 2, 0},
    {0xEC, (uint8_t[]) { 0x00, 0x20 }, 2, 0},
    {0xED, (uint8_t[]) { 0x00, 0x14 }, 2, 0},
    {0xEE, (uint8_t[]) { 0x00, 0xFF }, 2, 0},
    {0xEF, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xF8, (uint8_t[]) { 0x00, 0xFF }, 2, 0},
    {0xF9, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xFA, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xFB, (uint8_t[]) { 0x00, 0x30 }, 2, 0},
    {0xFC, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xFD, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xFE, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xFF, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x60, (uint8_t[]) { 0x00, 0x42 }, 2, 0},
    {0x61, (uint8_t[]) { 0x00, 0xE0 }, 2, 0},
    {0x62, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x63, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x64, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x65, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x66, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x67, (uint8_t[]) { 0x00, 0x03 }, 2, 0},
    {0x68, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x69, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x6A, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x6B, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x70, (uint8_t[]) { 0x00, 0x42 }, 2, 0},
    {0x71, (uint8_t[]) { 0x00, 0xE0 }, 2, 0},
    {0x72, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x73, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x74, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x75, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x76, (uint8_t[]) { 0x00, 0x40 }, 2, 0},
    {0x77, (uint8_t[]) { 0x00, 0x03 }, 2, 0},
    {0x78, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x79, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x7A, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x7B, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x80, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0x81, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x82, (uint8_t[]) { 0x00, 0x05 }, 2, 0},
    {0x83, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x84, (uint8_t[]) { 0x00, 0xDD }, 2, 0},
    {0x85, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x86, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x87, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x88, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0x89, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x8A, (uint8_t[]) { 0x00, 0x07 }, 2, 0},
    {0x8B, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x8C, (uint8_t[]) { 0x00, 0xDF }, 2, 0},
    {0x8D, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x8E, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x8F, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x90, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0x91, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x92, (uint8_t[]) { 0x00, 0x09 }, 2, 0},
    {0x93, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x94, (uint8_t[]) { 0x00, 0xE1 }, 2, 0},
    {0x95, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x96, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x97, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x98, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0x99, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x9A, (uint8_t[]) { 0x00, 0x0B }, 2, 0},
    {0x9B, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0x9C, (uint8_t[]) { 0x00, 0xE3 }, 2, 0},
    {0x9D, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x9E, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0x9F, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xA0, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0xA1, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xA2, (uint8_t[]) { 0x00, 0x04 }, 2, 0},
    {0xA3, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0xA4, (uint8_t[]) { 0x00, 0xDC }, 2, 0},
    {0xA5, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xA6, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xA7, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xA8, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0xA9, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xAA, (uint8_t[]) { 0x00, 0x06 }, 2, 0},
    {0xAB, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0xAC, (uint8_t[]) { 0x00, 0xDE }, 2, 0},
    {0xAD, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xAE, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xAF, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xB0, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0xB1, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xB2, (uint8_t[]) { 0x00, 0x08 }, 2, 0},
    {0xB3, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0xB4, (uint8_t[]) { 0x00, 0xE0 }, 2, 0},
    {0xB5, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xB6, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xB7, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xB8, (uint8_t[]) { 0x00, 0x48 }, 2, 0},
    {0xB9, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBA, (uint8_t[]) { 0x00, 0x0A }, 2, 0},
    {0xBB, (uint8_t[]) { 0x00, 0x02 }, 2, 0},
    {0xBC, (uint8_t[]) { 0x00, 0xE2 }, 2, 0},
    {0xBD, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBE, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xBF, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xC0, (uint8_t[]) { 0x00, 0x22 }, 2, 0},
    {0xC1, (uint8_t[]) { 0x00, 0x98 }, 2, 0},
    {0xC2, (uint8_t[]) { 0x00, 0x65 }, 2, 0},
    {0xC3, (uint8_t[]) { 0x00, 0x74 }, 2, 0},
    {0xC4, (uint8_t[]) { 0x00, 0x47 }, 2, 0},
    {0xC5, (uint8_t[]) { 0x00, 0x56 }, 2, 0},
    {0xC6, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xC7, (uint8_t[]) { 0x00, 0xBA }, 2, 0},
    {0xC8, (uint8_t[]) { 0x00, 0xAB }, 2, 0},
    {0xC9, (uint8_t[]) { 0x00, 0x33 }, 2, 0},
    {0xD0, (uint8_t[]) { 0x00, 0x11 }, 2, 0},
    {0xD1, (uint8_t[]) { 0x00, 0x98 }, 2, 0},
    {0xD2, (uint8_t[]) { 0x00, 0x65 }, 2, 0},
    {0xD3, (uint8_t[]) { 0x00, 0x74 }, 2, 0},
    {0xD4, (uint8_t[]) { 0x00, 0x47 }, 2, 0},
    {0xD5, (uint8_t[]) { 0x00, 0x56 }, 2, 0},
    {0xD6, (uint8_t[]) { 0x00, 0x00 }, 2, 0},
    {0xD7, (uint8_t[]) { 0x00, 0xBA }, 2, 0},
    {0xD8, (uint8_t[]) { 0x00, 0xAB }, 2, 0},
    {0xD9, (uint8_t[]) { 0x00, 0x33 }, 2, 0},
    {0xF3, (uint8_t[]) { 0x00, 0x01 }, 2, 0},
    {0xF0, (uint8_t[]) { 0x00, 0x00 }, 2, 0},

    {0x21, (uint8_t[]) { 0x00 }, 1, 0},
    {0x11, (uint8_t[]) { 0x00 }, 0, 120},
    {0x29, (uint8_t[]) { 0x00 }, 0, 120},

    // {0x00, (uint8_t[]) { 0x00 }, 1, 0},
    // {0x1C, (uint8_t[]) { 0x00 }, 0, 50},


    //============ Gamma END===========
};

static esp_err_t panel_h013a08_del(esp_lcd_panel_t *panel) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;

    if (h013a08->reset_gpio_num >= 0) {
        gpio_reset_pin(h013a08->reset_gpio_num);
    }
    // Delete MIPI DPI panel
    h013a08->del(panel);
    ESP_LOGD(TAG, "del h013a08 panel @%p", h013a08);
    free(h013a08);

    return ESP_OK;
}

static esp_err_t panel_h013a08_init(esp_lcd_panel_t *panel) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;
    const h013a08_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    bool is_command0_enable = false;
    bool is_cmd_overwritten = false;

    // // The ID register is on the CMD_Page 1
    // uint8_t ID1, ID2, ID3;
    // esp_lcd_panel_io_tx_param(io, 0xFF, (uint8_t[]) { 0x98, 0x81, 0x01 }, 3);
    // esp_lcd_panel_io_rx_param(io, 0x00, &ID1, 1);
    // esp_lcd_panel_io_rx_param(io, 0x01, &ID2, 1);
    // esp_lcd_panel_io_rx_param(io, 0x02, &ID3, 1);
    // ESP_LOGI(TAG, "LCD ID: %02X %02X %02X", ID1, ID2, ID3);

    // vendor specific initialization, it can be different between manufacturers
    // should consult the LCD supplier for initialization sequence code
    if (h013a08->init_cmds) {
        init_cmds = h013a08->init_cmds;
        init_cmds_size = h013a08->init_cmds_size;
    } else {
        init_cmds = vendor_specific_init_default;
        init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(h013a08_lcd_init_cmd_t);
    }

    for (int i = 0; i < init_cmds_size; i++) {
        // Check if the command has been used or conflicts with the internal
        if (is_command0_enable && init_cmds[i].data_bytes > 0) {
            switch (init_cmds[i].cmd) {
            case LCD_CMD_MADCTL:
                is_cmd_overwritten = true;
                h013a08->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
                break;
            case LCD_CMD_COLMOD:
                is_cmd_overwritten = true;
                h013a08->colmod_val = ((uint8_t *)init_cmds[i].data)[0];
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
        ESP_LOGW(TAG, "send commands %d/%d", i, init_cmds_size);

    }
    ESP_LOGI(TAG, "send init commands success");

    ESP_RETURN_ON_ERROR(h013a08->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_h013a08_reset(esp_lcd_panel_t *panel) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;

    // Perform hardware reset
    if (h013a08->reset_gpio_num >= 0) {
        gpio_set_level(h013a08->reset_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
        gpio_set_level(h013a08->reset_gpio_num, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(h013a08->reset_gpio_num, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    } else if (io) { // Perform software reset
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

static esp_err_t panel_h013a08_invert_color(esp_lcd_panel_t *panel, bool invert_color_data) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;
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

static esp_err_t panel_h013a08_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;
    uint8_t madctl_val = h013a08->madctl_val;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    // Control mirror through LCD command
    if (mirror_x) {
        madctl_val |= h013a08_CMD_GS_BIT;
    } else {
        madctl_val &= ~h013a08_CMD_GS_BIT;
    }
    if (mirror_y) {
        madctl_val |= h013a08_CMD_SS_BIT;
    } else {
        madctl_val &= ~h013a08_CMD_SS_BIT;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]) {
        madctl_val
    }, 1), TAG, "send command failed");
    h013a08->madctl_val = madctl_val;

    return ESP_OK;
}

static esp_err_t panel_h013a08_disp_on_off(esp_lcd_panel_t *panel, bool on_off) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;
    int command = 0;

    if (on_off) {
        command = LCD_CMD_DISPON;
    } else {
        command = LCD_CMD_DISPOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");
    return ESP_OK;
}

static esp_err_t panel_h013a08_sleep(esp_lcd_panel_t *panel, bool sleep) {
    h013a08_panel_t *h013a08 = (h013a08_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = h013a08->io;
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
