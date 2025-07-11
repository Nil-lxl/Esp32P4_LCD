/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

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


static i2c_master_bus_handle_t i2c_bus_handle;
static esp_lcd_touch_handle_t touch_handle;
static SemaphoreHandle_t at_send_cmd_sem;

esp_err_t touch_init() {
    i2c_master_bus_config_t i2c_bus_cfg = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .i2c_port = I2C_NUM_0,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_handle));
    const esp_lcd_touch_config_t touch_config = {
        .x_max = MIPI_DSI_LCD_H_RES,
        .y_max = MIPI_DSI_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC,
        .int_gpio_num = GPIO_NUM_NC,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 1,
            .mirror_y = 1,
        },
    };
    esp_lcd_panel_io_handle_t touch_io_handle;
    esp_lcd_panel_io_i2c_config_t touch_io_cfg = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    touch_io_cfg.scl_speed_hz = 400000;

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2(i2c_bus_handle, &touch_io_cfg, &touch_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(touch_io_handle, &touch_config, &touch_handle));
    return ESP_OK;
}

static void enable_dsi_phy_power(void) {
    // Turn on the power for MIPI DSI PHY, so it can go from "No Power" state to "Shutdown" state
    esp_ldo_channel_handle_t ldo_mipi_phy = NULL;
#ifdef MIPI_DSI_PHY_PWR_LDO_CHAN
    esp_ldo_channel_config_t ldo_mipi_phy_config = {
        .chan_id = MIPI_DSI_PHY_PWR_LDO_CHAN,
        .voltage_mv = MIPI_DSI_PHY_PWR_LDO_VOLTAGE_MV,
    };
    ESP_ERROR_CHECK(esp_ldo_acquire_channel(&ldo_mipi_phy_config, &ldo_mipi_phy));
    ESP_LOGI(TAG, "MIPI DSI PHY Powered on");
#endif
}

static void init_lcd_backlight(void) {
#if PIN_NUM_BK_LIGHT >= 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
#endif
}

static void set_lcd_backlight(uint32_t level) {
#if PIN_NUM_BK_LIGHT >= 0
    gpio_set_level(PIN_NUM_BK_LIGHT, level);
#endif
}

static esp_lcd_panel_handle_t mipi_dsi_panel;
static esp_lcd_panel_io_handle_t panel_io_handle;

esp_err_t mipi_lcd_init() {
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
        .dpi_clock_freq_mhz = MIPI_DSI_PIXEL_CLK_MHZ,
        .in_color_format = LCD_COLOR_FMT_RGB888,
        .num_fbs = 2,   //double frame buffer
        .video_timing = {
            .h_size = MIPI_DSI_LCD_H_RES,
            .v_size = MIPI_DSI_LCD_V_RES,
            .hsync_back_porch = MIPI_DSI_LCD_HBP,
            .hsync_pulse_width = MIPI_DSI_LCD_HSYNC,
            .hsync_front_porch = MIPI_DSI_LCD_HFP,
            .vsync_back_porch = MIPI_DSI_LCD_VBP,
            .vsync_pulse_width = MIPI_DSI_LCD_VSYNC,
            .vsync_front_porch = MIPI_DSI_LCD_VFP,
        },
#if CONFIG_EXAMPLE_USE_DMA2D_COPY_FRAME
        .flags.use_dma2d = true, // use DMA2D to copy draw buffer into frame buffer
#endif
    };

#if CONFIG_EXAMPLE_LCD_USE_EK79007
    ek79007_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ek79007(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));
#elif CONFIG_EXAMPLE_LCD_USE_ST7703
    st7703_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));

#elif CONFIG_EXAMPLE_LCD_USE_ST7703_720x720
    st7703_720_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7703_720(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));

#elif CONFIG_EXAMPLE_LCD_USE_JD9365
    jd9365_vendor_config_t vendor_config = {
        .mipi_config = {
            .dsi_bus = dsi_bus_handle,
            .dpi_config = &dpi_config,
        },
    };
    esp_lcd_panel_dev_config_t lcd_dev_config = {
        .reset_gpio_num = EXAMPLE_PIN_NUM_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 24,
        .vendor_config = &vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9365(panel_io_handle, &lcd_dev_config, &mipi_dsi_panel));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(mipi_dsi_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(mipi_dsi_panel));
    // ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(mipi_dsi_panel,true));

    return ESP_OK;
}

static lv_display_t *display;
static lv_indev_t *lv_touch_indev;
esp_err_t lvgl_init() {

    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,
        .task_stack = 24 * 1024,
        .task_affinity = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5,

    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");
    uint32_t buf_size = MIPI_DSI_LCD_H_RES * MIPI_DSI_LCD_V_RES * 5;

    const lvgl_port_display_cfg_t disp_config = {
        .panel_handle = mipi_dsi_panel,
        .io_handle = panel_io_handle,
        .buffer_size = buf_size,
        .double_buffer = 1,
        .hres = MIPI_DSI_LCD_H_RES,
        .vres = MIPI_DSI_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB888,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
            .direct_mode = true,
            // .full_refresh = true,
            .swap_bytes = false,
            // .sw_rotate = true,
        },
    };
    const lvgl_port_display_dsi_cfg_t dsi_config = {
        .flags = {
            .avoid_tearing = true,
        }
    };
    display = lvgl_port_add_disp_dsi(&disp_config, &dsi_config);
#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = display,
        .handle = touch_handle,
    };
    lv_touch_indev = lvgl_port_add_touch(&touch_cfg);

#endif
    return ESP_OK;
}

extern void example_lvgl_demo_ui(lv_display_t *disp);
extern void sdio_at_startup(void);

void app_main(void) {

    enable_dsi_phy_power();

    init_lcd_backlight();
    set_lcd_backlight(LCD_BK_LIGHT_OFF_LEVEL);
#ifdef CONFIG_EXAMPLE_LCD_USE_TOUCH_ENABLED
    ESP_ERROR_CHECK(touch_init());
#endif    
    ESP_ERROR_CHECK(mipi_lcd_init());
    ESP_ERROR_CHECK(lvgl_init());
    set_lcd_backlight(LCD_BK_LIGHT_ON_LEVEL);

    lvgl_port_lock(0);
    example_lvgl_demo_ui(display);
    lvgl_port_unlock();

    sdio_at_startup();

}