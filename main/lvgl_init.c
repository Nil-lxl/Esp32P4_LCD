#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_lcd_touch.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lcd_config.h"
#include "lcd_touch.h"

static lv_display_t *display;
static esp_lcd_touch_handle_t touch_handle;
static lv_indev_t *lv_touch_indev;

static const char *TAG = "LVGL_INIT";

extern void example_lvgl_demo_ui(lv_display_t *disp);

esp_err_t lvgl_init(esp_lcd_panel_handle_t *panel, esp_lcd_panel_io_handle_t *panel_io_handle) {
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,
        .task_stack = 24 * 1024,
        .task_affinity = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5,

    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");
    uint32_t buf_size = LCD_H_RES * LCD_H_RES * 5;

    const lvgl_port_display_cfg_t disp_config = {
        .panel_handle = *panel,
#if CONFIG_LCD_USE_MIPI_INTERFACE
        .io_handle = *panel_io_handle,
#endif
        .buffer_size = buf_size,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT,
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

#if CONFIG_LCD_USE_MIPI_INTERFACE
    const lvgl_port_display_dsi_cfg_t dsi_config = {
        .flags = {
            .avoid_tearing = true,
        }
    };
    display = lvgl_port_add_disp_dsi(&disp_config, &dsi_config);
#elif CONFIG_LCD_USE_RGB_INTERFACE
    const lvgl_port_display_rgb_cfg_t rgb_cfg = {
        .flags = {
            .bb_mode = false,
            .avoid_tearing = true,
        }
    };
    display = lvgl_port_add_disp_rgb(&disp_config, &rgb_cfg);
#endif 

#ifdef CONFIG_LCD_USE_TOUCH_ENABLED
    lcd_touch_init(&touch_handle);

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = display,
        .handle = touch_handle,
    };
    lv_touch_indev = lvgl_port_add_touch(&touch_cfg);

#endif

    lvgl_port_lock(0);
    example_lvgl_demo_ui(display);
    lvgl_port_unlock();

    return ESP_OK;

}


