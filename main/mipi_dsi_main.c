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

extern esp_err_t mipi_lcd_start(void);
extern esp_err_t lvgl_start(void);


extern void sdio_at_startup(void);

void app_main(void) {

    ESP_ERROR_CHECK(mipi_lcd_start());
    ESP_ERROR_CHECK(lvgl_start());

}