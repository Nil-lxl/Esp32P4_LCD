//
// Created by Administrator on 25-2-21.
//

#ifndef TOUCH_UI_H
#define TOUCH_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "ui/ui_helpers.h"
#define UI_EXAMPLE_USE_IMGBTN  1

#define CONTAINER_WIDTH		320
#define CONTAINER_HEIGHT	240
#define MENU_ITEM_SIDE		64
#define MENU_ITEM_MARGIN	120
#define ITEM_COUNT			5

//************home page***************//
void touch_ui_home_init(void);
extern lv_obj_t* home_scr;

//SCREEN: TOUCH_SCREEN1
void ui_touch_screen1_init(void);
extern lv_obj_t* touch_screen1;

void ui_touch_screen2_init(void);
extern lv_obj_t* touch_screen2;

void toggle_screen1_event(lv_event_t* e);
void toggle_screen2_event(lv_event_t* e);
void back_home_event(lv_event_t* e);

//***********IMAGES************//
LV_IMG_DECLARE(img_app);
LV_IMG_DECLARE(img_email);
LV_IMG_DECLARE(img_setting);
LV_IMG_DECLARE(img_camera);
LV_IMG_DECLARE(img_home);
LV_IMG_DECLARE(img_more);
LV_IMG_DECLARE(home);
LV_IMG_DECLARE(email);
LV_IMG_DECLARE(notice);
LV_IMG_DECLARE(position);
LV_IMG_DECLARE(team);
LV_IMG_DECLARE(poweroff);
#ifdef __cplusplus
}
#endif //extern "C"

#endif //TOUCH_UI_H
