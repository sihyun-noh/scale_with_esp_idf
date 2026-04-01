#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN = 1,
    SCREEN_ID_KOR_THREE_AXIS = 2,
    SCREEN_ID_CALCULATOR = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *main;
    lv_obj_t *kor_three_axis;
    lv_obj_t *calculator;
    lv_obj_t *obj0;
    lv_obj_t *cmd_panel;
    lv_obj_t *____;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *obj7;
    lv_obj_t *feedback;
    lv_obj_t *obj8;
    lv_obj_t *obj9;
    lv_obj_t *both;
    lv_obj_t *obj10;
    lv_obj_t *run;
    lv_obj_t *obj11;
    lv_obj_t *obj12;
    lv_obj_t *obj13;
    lv_obj_t *obj14;
    lv_obj_t *obj15;
    lv_obj_t *obj16;
    lv_obj_t *obj17;
    lv_obj_t *obj18;
    lv_obj_t *obj19;
    lv_obj_t *obj20;
    lv_obj_t *obj21;
    lv_obj_t *obj22;
    lv_obj_t *obj23;
    lv_obj_t *obj24;
    lv_obj_t *obj25;
    lv_obj_t *obj26;
    lv_obj_t *obj27;
    lv_obj_t *st_1_0;
    lv_obj_t *st_1_1;
    lv_obj_t *st_1_2;
    lv_obj_t *st_1_3;
    lv_obj_t *st_1_4;
    lv_obj_t *st_1_5;
    lv_obj_t *st_1_6;
    lv_obj_t *st_1_7;
    lv_obj_t *obj28;
    lv_obj_t *cal;
    lv_obj_t *obj29;
    lv_obj_t *obj30;
    lv_obj_t *obj31;
    lv_obj_t *obj32;
    lv_obj_t *obj33;
    lv_obj_t *obj34;
    lv_obj_t *obj35;
    lv_obj_t *obj36;
    lv_obj_t *display_label;
    lv_obj_t *btn_clear;
    lv_obj_t *btn_negate;
    lv_obj_t *btn_percent;
    lv_obj_t *btn_divide;
    lv_obj_t *btn_7;
    lv_obj_t *btn_8;
    lv_obj_t *btn_9;
    lv_obj_t *btn_multiply;
    lv_obj_t *btn_4;
    lv_obj_t *btn_5;
    lv_obj_t *btn_6;
    lv_obj_t *btn_subtract;
    lv_obj_t *btn_1;
    lv_obj_t *btn_2;
    lv_obj_t *btn_3;
    lv_obj_t *btn_add;
    lv_obj_t *btn_0;
    lv_obj_t *btn_decimal;
    lv_obj_t *btn_equals;
    lv_obj_t *btn_enter;
    lv_obj_t *set_d_1;
    lv_obj_t *set_d_2;
    lv_obj_t *obj37;
    lv_obj_t *obj38;
    lv_obj_t *obj39;
    lv_obj_t *obj40;
    lv_obj_t *obj41;
} objects_t;

extern objects_t objects;

void create_screen_main();
void tick_screen_main();

void create_screen_kor_three_axis();
void tick_screen_kor_three_axis();

void create_screen_calculator();
void tick_screen_calculator();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/