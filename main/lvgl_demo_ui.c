/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "string.h"
#include "lvgl.h"
#include "esp_log.h"
#include "demos/lv_demos.h"

extern void wifi_ui(void);

void example_lvgl_demo_ui(lv_display_t *disp) {
    // lv_demo_benchmark();
    // lv_demo_widgets();

    lv_obj_t *scr = lv_scr_act();

    // lv_obj_t* logo_gif=lv_gif_create(scr);
    // lv_gif_set_src(logo_gif,"/logo.gif");
    // lv_obj_center(logo_gif);

    lv_obj_t *logo_png = lv_image_create(scr);
    lv_image_set_src(logo_png, "/logo.png");
    lv_obj_center(logo_png);
    


}




