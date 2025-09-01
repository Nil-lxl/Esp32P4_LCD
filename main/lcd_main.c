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

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_panel_ops.h"

#include "driver/gpio.h"

#include "lcd_driver.h"
#include "lcd_config.h"
#include "lcd_touch.h"

static esp_lcd_panel_handle_t lcd_panel;
static esp_lcd_panel_io_handle_t lcd_panel_io;

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

extern esp_err_t lvgl_init(esp_lcd_panel_handle_t *panel, esp_lcd_panel_io_handle_t *panel_io_handle);

void app_main(void) {

    init_lcd_backlight();

#if CONFIG_LCD_USE_MIPI_INTERFACE
    ESP_ERROR_CHECK(mipi_lcd_init(&lcd_panel, &lcd_panel_io));
#elif CONFIG_LCD_USE_RGB_INTERFACE
    ESP_ERROR_CHECK(rgb_lcd_init(&lcd_panel));
#endif 
    set_lcd_backlight(1);

    ESP_ERROR_CHECK(lvgl_init(&lcd_panel, &lcd_panel_io));

}