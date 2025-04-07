//
// Created by Administrator on 25-3-28.
//

#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#ifdef __cplusplus
    extern "C" { 
#endif

#include "lvgl.h"

void ui_screen_change(lv_obj_t **target,lv_scr_load_anim_t fademode,int speed,int delay,void(*target_init)(void));

#ifdef __cplusplus
}
#endif //exteren "C"

#endif //UI_HELPERS_H
