//
// Created by Administrator on 24-12-17.
//

#ifndef LV_DEMO_H
#define LV_DEMO_H

#ifdef __cplusplus
    extern "C" { 
#endif
#include "lvgl__lvgl/src/misc/lv_timer.h"

#define USE_IMG_MENU 0

#define MENU_PAGE 1
#define CONTENT_PAGE 2

#define MENU_CONTAINER_WIDHT 200
#define MENU_CONTAINER_HEIGHT 200

#define MENU_ITEM_COUNT 6
#define MENU_ITEM_WIDTH 140
#define MENU_ITEM_HEIGHT 30
#define MENU_ITEM_MARGIN_Y 80

static lv_timer_t* timer;
static lv_timer_t* page_timer;

static lv_obj_t* top_layer;
static lv_obj_t* scr;
static lv_obj_t* container;  // 菜单容器
static lv_obj_t* menu_item[MENU_ITEM_COUNT];
static const char* menu_item_text[MENU_ITEM_COUNT]={"Item1","Item2","Item3","Item4","Item5","setting"};

static lv_obj_t* enterBtn;
static lv_obj_t* backBtn;

static int page_level;
static int sum = 0;
static int prev_sum=0;
static int menu_seq=0;

void lv_menu();
void lv_menu_init(void);
void encoder_ui_init(void);

void timer_page_cb(lv_timer_t* timer);

void page1();
void page2();
void page3();
void page4();
void page5();
void page6();

#ifdef __cplusplus
}
#endif //exteren "C"

#endif //LV_DEMO_H
