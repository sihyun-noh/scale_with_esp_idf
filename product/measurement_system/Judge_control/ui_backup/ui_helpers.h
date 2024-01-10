// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_HELPERS_H
#define _SQUARELINE_PROJECT_UI_HELPERS_H

#include "ui.h"

typedef enum {
  ZONE_1 = 1,
  ZONE_2,
  ZONE_3,
  ZONE_4,
  ZONE_5,
  ZONE_6,
} ZONE;

#ifdef __cplusplus
extern "C" {
#endif

#define _UI_TEMPORARY_STRING_BUFFER_SIZE 32
#define _UI_BAR_PROPERTY_VALUE 0
#define _UI_BAR_PROPERTY_VALUE_WITH_ANIM 1
void _ui_bar_set_property(lv_obj_t *target, int id, int val);

#define _UI_BASIC_PROPERTY_POSITION_X 0
#define _UI_BASIC_PROPERTY_POSITION_Y 1
#define _UI_BASIC_PROPERTY_WIDTH 2
#define _UI_BASIC_PROPERTY_HEIGHT 3
void _ui_basic_set_property(lv_obj_t *target, int id, int val);

#define _UI_DROPDOWN_PROPERTY_SELECTED 0
void _ui_dropdown_set_property(lv_obj_t *target, int id, int val);

#define _UI_IMAGE_PROPERTY_IMAGE 0
void _ui_image_set_property(lv_obj_t *target, int id, uint8_t *val);

#define _UI_LABEL_PROPERTY_TEXT 0
void _ui_label_set_property(lv_obj_t *target, int id, char *val);

#define _UI_ROLLER_PROPERTY_SELECTED 0
#define _UI_ROLLER_PROPERTY_SELECTED_WITH_ANIM 1
void _ui_roller_set_property(lv_obj_t *target, int id, int val);

#define _UI_SLIDER_PROPERTY_VALUE 0
#define _UI_SLIDER_PROPERTY_VALUE_WITH_ANIM 1
void _ui_slider_set_property(lv_obj_t *target, int id, int val);

void _ui_screen_change(lv_obj_t *target, lv_scr_load_anim_t fademode, int spd, int delay);

void _ui_arc_increment(lv_obj_t *target, int val);

void _ui_bar_increment(lv_obj_t *target, int val, int anm);

void _ui_slider_increment(lv_obj_t *target, int val, int anm);

#define _UI_MODIFY_FLAG_ADD 0
#define _UI_MODIFY_FLAG_REMOVE 1
#define _UI_MODIFY_FLAG_TOGGLE 2
void _ui_flag_modify(lv_obj_t *target, int32_t flag, int value);

#define _UI_MODIFY_STATE_ADD 0
#define _UI_MODIFY_STATE_REMOVE 1
#define _UI_MODIFY_STATE_TOGGLE 2
void _ui_state_modify(lv_obj_t *target, int32_t state, int value);

void _ui_opacity_set(lv_obj_t *target, int val);

void _ui_anim_callback_set_x(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_y(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_width(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_height(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_opacity(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_image_zoom(lv_anim_t *a, int32_t v);

void _ui_anim_callback_set_image_angle(lv_anim_t *a, int32_t v);

int32_t _ui_anim_callback_get_x(lv_anim_t *a);

int32_t _ui_anim_callback_get_y(lv_anim_t *a);

int32_t _ui_anim_callback_get_width(lv_anim_t *a);

int32_t _ui_anim_callback_get_height(lv_anim_t *a);

int32_t _ui_anim_callback_get_opacity(lv_anim_t *a);

int32_t _ui_anim_callback_get_image_zoom(lv_anim_t *a);

int32_t _ui_anim_callback_get_image_angle(lv_anim_t *a);

void _ui_arc_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *prefix, char *postfix);

void _ui_slider_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *prefix, char *postfix);

void _ui_checked_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *txt_on, char *txt_off);

void warnning_msgbox(char *message);

void enable_buttons(void);
void disable_buttons(void);

void enable_start_button(void);
void disable_start_button(void);

void set_zone_status(ZONE zone, bool start);
void set_zone_number(ZONE zone, bool start);
void set_zone_flow_value(ZONE zone, int flow_value);
char *get_checked_zones(void);
void reset_settings(void);
void add_operation_list(const char *op_msg);
void syscfg_get_flow_value(ZONE zone, char *flow_value, size_t value_len);
void syscfg_set_flow_value(ZONE zone, char *flow_value);
void disable_zone(ZONE zone);
void update_zones(void);
void get_removed_device_list(char *device_list, size_t buf_len);

#ifdef __cplusplus
}
#endif

#endif
