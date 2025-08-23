#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include "sdkconfig.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_ldo_regulator.h"

#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lcd_config.h"

static lv_display_t *display;
static lv_indev_t *lv_touch_indev;

extern esp_lcd_panel_handle_t mipi_dsi_panel;
extern esp_lcd_panel_io_handle_t panel_io_handle;

static esp_err_t lvgl_init() {
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

esp_err_t lvgl_start(void) {
    ESP_ERROR_CHECK(lvgl_init());

    lvgl_port_lock(0);
    example_lvgl_demo_ui(display);
    lvgl_port_unlock();

    return ESP_OK;
}

