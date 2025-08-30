#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_ldo_regulator.h"

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_touch.h"
#include "esp_lcd_touch_gt911.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"

#include "lcd_config.h"
#include "lcd_driver.h"

#if CONFIG_LCD_USE_MIPI_INTERFACE

static const char *TAG = "Mipi_driver";

static void enable_dsi_phy_power(void) {
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
#ifdef MIPI_PHY_LDO_CHANNEL
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = MIPI_PHY_LDO_CHANNEL,
        .voltage_mv = MIPI_PHY_LDO_VOLTAGE,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
#endif
}

static esp_lcd_panel_handle_t mipi_dsi_panel;
static esp_lcd_panel_io_handle_t panel_io_handle;

esp_err_t mipi_lcd_init(esp_lcd_panel_handle_t *panel_handle, esp_lcd_panel_io_handle_t *io_handle) {
    enable_dsi_phy_power();
    // create MIPI DSI bus first, it will initialize the DSI PHY as well
    esp_lcd_dsi_bus_handle_t dsi_bus_handle;
    esp_lcd_dsi_bus_config_t dsi_bus_config = {
        .bus_id = 0,
        .num_data_lanes = MIPI_DSI_LANE_NUM,
        .phy_clk_src = MIPI_DSI_PHY_CLK_SRC_DEFAULT,
        .lane_bit_rate_mbps = MIPI_DSI_LANE_BITRATE_MBPS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_dsi_bus(&dsi_bus_config, &dsi_bus_handle));
    ESP_LOGI(TAG, "Install MIPI DSI LCD control IO");
    // we use DBI interface to send LCD commands and parameters
    esp_lcd_dbi_io_config_t dbi_config = {
        .virtual_channel = 0,
        .lcd_cmd_bits = 8,   // according to the LCD spec
        .lcd_param_bits = 8, // according to the LCD spec
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_dbi(dsi_bus_handle, &dbi_config, &panel_io_handle));

    ESP_LOGI(TAG, "Install MIPI DSI LCD data panel");

    esp_lcd_dpi_panel_config_t dpi_config = {
        .virtual_channel = 0,
        .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,
        .dpi_clock_freq_mhz = LCD_PIXEL_CLK_MHZ,
        .in_color_format = LCD_COLOR_FMT_RGB888,
        .num_fbs = 2,   //double frame buffer
        .video_timing = {
            .h_size = LCD_H_RES,
            .v_size = LCD_V_RES,
            .hsync_back_porch = LCD_HBP,
            .hsync_pulse_width = LCD_HSYNC,
            .hsync_front_porch = LCD_HFP,
            .vsync_back_porch = LCD_VBP,
            .vsync_pulse_width = LCD_VSYNC,
            .vsync_front_porch = LCD_VFP,
        },
#if CONFIG_EXAMPLE_USE_DMA2D_COPY_FRAME
        .flags.use_dma2d = true, // use DMA2D to copy draw buffer into frame buffer
#endif
    };

#if CONFIG_LCD_USE_PANEL_EK79007
    ek79007_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ek79007(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));
#elif CONFIG_LCD_USE_PANEL_H070B13
    h070b13_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_h070b13(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));
#elif CONFIG_LCD_USE_PANEL_ST7703
    st7703_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(mipi_dsi_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(mipi_dsi_panel));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(mipi_dsi_panel,true));

    if (panel_handle) {
        *panel_handle = mipi_dsi_panel;
    }
    if (panel_io_handle) {
        *io_handle = panel_io_handle;
    }

    return ESP_OK;
}

#endif