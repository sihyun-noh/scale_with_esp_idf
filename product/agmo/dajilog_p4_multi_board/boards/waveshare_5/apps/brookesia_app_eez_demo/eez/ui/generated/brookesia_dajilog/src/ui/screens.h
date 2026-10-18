#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_STARTUP_ANIMATION = 1,
    SCREEN_ID_MAIN = 2,
    SCREEN_ID_NUMERIC_INPUT = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *startup_animation;
    lv_obj_t *main;
    lv_obj_t *numeric_input;
    lv_obj_t *startup_top_accent;
    lv_obj_t *startup_brand_label;
    lv_obj_t *startup_subtitle_label;
    lv_obj_t *startup_hopper_backdrop;
    lv_obj_t *startup_hopper_image;
    lv_obj_t *startup_particle_1;
    lv_obj_t *startup_particle_2;
    lv_obj_t *startup_particle_3;
    lv_obj_t *startup_particle_4;
    lv_obj_t *startup_particle_5;
    lv_obj_t *startup_particle_6;
    lv_obj_t *startup_particle_7;
    lv_obj_t *startup_particle_8;
    lv_obj_t *startup_particle_9;
    lv_obj_t *startup_percent_0;
    lv_obj_t *startup_percent_25;
    lv_obj_t *startup_percent_50;
    lv_obj_t *startup_percent_75;
    lv_obj_t *startup_percent_100;
    lv_obj_t *startup_dispensed_label;
    lv_obj_t *startup_progress_track;
    lv_obj_t *startup_progress_fill;
    lv_obj_t *startup_status_label;
    lv_obj_t *startup_module_label;
    lv_obj_t *startup_footer_label;
    lv_obj_t *work_screen_card;
    lv_obj_t *work_screen_title_label;
    lv_obj_t *emergency_stop_button;
    lv_obj_t *emergency_stop_button_label;
    lv_obj_t *drive_mode_label;
    lv_obj_t *actual_speed_title_label;
    lv_obj_t *actual_speed_value_label;
    lv_obj_t *separator_speed;
    lv_obj_t *front_target_title_label;
    lv_obj_t *front_target_value_label;
    lv_obj_t *separator_front;
    lv_obj_t *rear_target_title_label;
    lv_obj_t *rear_target_value_label;
    lv_obj_t *separator_rear;
    lv_obj_t *correction_status_label;
    lv_obj_t *operation_start_button;
    lv_obj_t *operation_start_button_label;
    lv_obj_t *hopper_detail_button;
    lv_obj_t *hopper_detail_button_label;
    lv_obj_t *operation_pause_button;
    lv_obj_t *operation_pause_button_label;
    lv_obj_t *section_end_button;
    lv_obj_t *section_end_button_label;
    lv_obj_t *status_accent;
    lv_obj_t *operation_hint_label;
    lv_obj_t *operation_ready_label;
    lv_obj_t *manual_target_title_label;
    lv_obj_t *manual_target_input;
    lv_obj_t *manual_target_unit_label;
    lv_obj_t *manual_target_help_label;
    lv_obj_t *numeric_input_card;
    lv_obj_t *numeric_input_title_label;
    lv_obj_t *manual_target_editor;
    lv_obj_t *numeric_input_unit_label;
    lv_obj_t *manual_target_keypad;
    lv_obj_t *manual_target_done_button;
    lv_obj_t *manual_target_done_button_label;
    lv_obj_t *numeric_input_help_label;
} objects_t;

extern objects_t objects;

void create_screen_startup_animation();
void tick_screen_startup_animation();

void create_screen_main();
void tick_screen_main();

void create_screen_numeric_input();
void tick_screen_numeric_input();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/