/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "string.h"
#include "lvgl.h"
#include "esp_log.h"
#include "demos/lv_demos.h"
#include "src/ui/ui.h"

#include "esp_serial_slave_link/essl_sdio.h"
#include "at_sdio_cmd.h"

extern void wifi_ui(void);

void example_lvgl_demo_ui(lv_display_t *disp) {
    // lv_demo_benchmark();
    // lv_demo_widgets();
    // UI_init();
    wifi_ui();
}




