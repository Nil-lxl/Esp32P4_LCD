//
// Created by Administrator on 25-2-21.
//

#ifndef TOUCH_UI_H
#define TOUCH_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "../src/ui/ui_helpers.h"
#define UI_EXAMPLE_USE_IMGBTN  0

#define CONTAINER_WIDTH		480
#define CONTAINER_HEIGHT	320
#define NAV_ITEM_SIDE		64
#define NAV_ITEM_MARGIN		60
#define NAV_COUNT			3
#define ITEM_COUNT			8


//************home page***************//
void touch_ui_home_init(void);
extern lv_obj_t* home_scr;

//SCREEN: TOUCH_SCREEN1
void ui_touch_screen1_init(void);
extern lv_obj_t* touch_screen1;

void ui_touch_screen2_init(void);
extern lv_obj_t* touch_screen2;

void ui_touch_screen3_init(void);
extern lv_obj_t* touch_screen3;

extern lv_obj_t* label_time;
extern lv_obj_t* label_date;
void update_localtime_cb(lv_timer_t* timer);
void toggle_screen1_event(lv_event_t* e);
void toggle_screen2_event(lv_event_t* e);
void toggle_screen3_event(lv_event_t* e);
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
