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

#define CONTAINER_WIDTH		480
#define CONTAINER_HEIGHT	320
#define NAV_ITEM_WIDTH		64
#define NAV_ITEM_HEIGHT		90
#define NAV_ITEM_MARGIN		60
#define NAV_COUNT			3
#define ITEM_COUNT			8

static bool show_exit;
extern lv_obj_t* exit_btn;

//************home page***************//
void touch_ui_home_init(void);
extern lv_obj_t* home_scr;

//SCREEN: TOUCH_SCREEN1
void ui_screen1_init(void);
extern lv_obj_t* touch_screen1;

void ui_screen2_init(void);
extern lv_obj_t* touch_screen2;

void ui_screen3_init(void);
extern lv_obj_t* touch_screen3;

void image_screen_init(void);
extern lv_obj_t* image_screen;

void calculator_screen_init(void);
extern lv_obj_t* calculator_screen;

extern lv_obj_t* label_time;
extern lv_obj_t* label_date;
void update_localtime_cb(lv_timer_t* timer);

void toggle_screen1_event(lv_event_t* e);
void toggle_screen2_event(lv_event_t* e);
void toggle_screen3_event(lv_event_t* e);
void toggle_image_event(lv_event_t* e);
void toggle_calculator_event(lv_event_t* e);

void back_home_event(lv_event_t* e);

//***********Button ICONS************//
LV_IMG_DECLARE(car);
LV_IMG_DECLARE(HOME);
LV_IMG_DECLARE(EMAIL);
LV_IMG_DECLARE(EMAIL_1);
LV_IMG_DECLARE(LOCATION);
LV_IMG_DECLARE(SECURE);
LV_IMG_DECLARE(TIME);
LV_IMG_DECLARE(WALLET);
LV_IMG_DECLARE(WIFI);
LV_IMG_DECLARE(INFO);
LV_IMG_DECLARE(INFO_1);
LV_IMG_DECLARE(BLUETOOTH);
LV_IMG_DECLARE(CALENDAR);
LV_IMG_DECLARE(CALL)
LV_IMG_DECLARE(CAMERA);
LV_IMG_DECLARE(CHAT);
LV_IMG_DECLARE(HOURS);
LV_IMG_DECLARE(HOURS_1);
LV_IMG_DECLARE(back);
LV_IMG_DECLARE(CHAT);
LV_IMG_DECLARE(CHAT_1);
LV_IMG_DECLARE(CONTACT_FORM);
LV_IMG_DECLARE(CONTACTS);
LV_IMG_DECLARE(EMBEDDED);
LV_IMG_DECLARE(GLOBE);
LV_IMG_DECLARE(CALCULATOR);
LV_IMG_DECLARE(IMAGE);
LV_IMG_DECLARE(MONITOR);
LV_IMG_DECLARE(MONITORING);
LV_IMG_DECLARE(EXIT);
LV_IMG_DECLARE(EXIT_SMALL);
//*****************Bg images***************//
LV_IMG_DECLARE(bg_1);
LV_IMG_DECLARE(bg_2);
LV_IMG_DECLARE(bg_4);

LV_IMG_DECLARE(eye_gif1);
// LV_IMG_DECLARE();
#ifdef __cplusplus
}
#endif //extern "C"

#endif //TOUCH_UI_H
