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
    SCREEN_ID_RIDGE_SETUP = 3,
    SCREEN_ID_PREPARATION = 4,
    SCREEN_ID_WORK = 5,
    SCREEN_ID_WEIGHING_TEST = 6,
    SCREEN_ID_NUMERIC_INPUT = 7,
    _SCREEN_ID_LAST = 7
};

typedef struct _objects_t {
    lv_obj_t *startup_animation;
    lv_obj_t *main;
    lv_obj_t *ridge_setup;
    lv_obj_t *preparation;
    lv_obj_t *work;
    lv_obj_t *weighing_test;
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
    lv_obj_t *main_app_title;
    lv_obj_t *main_status;
    lv_obj_t *main_stop;
    lv_obj_t *main_stop_label;
    lv_obj_t *main_tab_ridge;
    lv_obj_t *main_tab_ridge_label;
    lv_obj_t *main_tab_line_ridge;
    lv_obj_t *main_tab_prescription;
    lv_obj_t *main_tab_prescription_label;
    lv_obj_t *main_tab_line_prescription;
    lv_obj_t *main_tab_preparation;
    lv_obj_t *main_tab_preparation_label;
    lv_obj_t *main_tab_line_preparation;
    lv_obj_t *main_tab_work;
    lv_obj_t *main_tab_work_label;
    lv_obj_t *main_tab_line_work;
    lv_obj_t *main_tab_weighing;
    lv_obj_t *main_tab_weighing_label;
    lv_obj_t *main_tab_line_weighing;
    lv_obj_t *main_content;
    lv_obj_t *main_left_title;
    lv_obj_t *main_divider;
    lv_obj_t *main_right_title;
    lv_obj_t *main_ridge_label;
    lv_obj_t *main_ridge_value;
    lv_obj_t *main_count_label;
    lv_obj_t *recipe_ridge_count_input;
    lv_obj_t *main_count_unit;
    lv_obj_t *main_width_label;
    lv_obj_t *recipe_ridge_width_input;
    lv_obj_t *main_width_unit;
    lv_obj_t *main_length_label;
    lv_obj_t *recipe_actual_length_input;
    lv_obj_t *main_length_unit;
    lv_obj_t *main_area_only;
    lv_obj_t *main_area_only_label;
    lv_obj_t *main_area_furrow;
    lv_obj_t *main_area_furrow_label;
    lv_obj_t *main_all;
    lv_obj_t *main_all_label;
    lv_obj_t *main_split;
    lv_obj_t *main_split_label;
    lv_obj_t *main_apply_length;
    lv_obj_t *main_apply_length_label;
    lv_obj_t *main_north_label;
    lv_obj_t *main_north_value;
    lv_obj_t *main_south_label;
    lv_obj_t *main_south_value;
    lv_obj_t *main_temp_save;
    lv_obj_t *main_temp_save_label;
    lv_obj_t *main_copy;
    lv_obj_t *main_copy_label;
    lv_obj_t *main_nitrogen;
    lv_obj_t *main_nitrogen_label;
    lv_obj_t *main_direct;
    lv_obj_t *main_direct_label;
    lv_obj_t *main_nitrogen_amount_label;
    lv_obj_t *recipe_nitrogen_rate_input;
    lv_obj_t *main_nitrogen_unit;
    lv_obj_t *main_upper_ratio_label;
    lv_obj_t *main_upper_ratio;
    lv_obj_t *main_lower_ratio_label;
    lv_obj_t *main_lower_ratio;
    lv_obj_t *main_upper_product_label;
    lv_obj_t *main_upper_product;
    lv_obj_t *main_lower_product_label;
    lv_obj_t *main_lower_product;
    lv_obj_t *main_check_amount;
    lv_obj_t *main_check_amount_label;
    lv_obj_t *main_save_settings;
    lv_obj_t *main_save_settings_label;
    lv_obj_t *ridge_app_title;
    lv_obj_t *ridge_status;
    lv_obj_t *ridge_stop;
    lv_obj_t *ridge_stop_label;
    lv_obj_t *ridge_tab_ridge;
    lv_obj_t *ridge_tab_ridge_label;
    lv_obj_t *ridge_tab_line_ridge;
    lv_obj_t *ridge_tab_prescription;
    lv_obj_t *ridge_tab_prescription_label;
    lv_obj_t *ridge_tab_line_prescription;
    lv_obj_t *ridge_tab_preparation;
    lv_obj_t *ridge_tab_preparation_label;
    lv_obj_t *ridge_tab_line_preparation;
    lv_obj_t *ridge_tab_work;
    lv_obj_t *ridge_tab_work_label;
    lv_obj_t *ridge_tab_line_work;
    lv_obj_t *ridge_tab_weighing;
    lv_obj_t *ridge_tab_weighing_label;
    lv_obj_t *ridge_tab_line_weighing;
    lv_obj_t *ridge_content;
    lv_obj_t *ridge_content_title;
    lv_obj_t *ridge_placeholder;
    lv_obj_t *ridge_guide;
    lv_obj_t *prep_app_title;
    lv_obj_t *prep_status;
    lv_obj_t *prep_stop;
    lv_obj_t *prep_stop_label;
    lv_obj_t *prep_tab_ridge;
    lv_obj_t *prep_tab_ridge_label;
    lv_obj_t *prep_tab_line_ridge;
    lv_obj_t *prep_tab_prescription;
    lv_obj_t *prep_tab_prescription_label;
    lv_obj_t *prep_tab_line_prescription;
    lv_obj_t *prep_tab_preparation;
    lv_obj_t *prep_tab_preparation_label;
    lv_obj_t *prep_tab_line_preparation;
    lv_obj_t *prep_tab_work;
    lv_obj_t *prep_tab_work_label;
    lv_obj_t *prep_tab_line_work;
    lv_obj_t *prep_tab_weighing;
    lv_obj_t *prep_tab_weighing_label;
    lv_obj_t *prep_tab_line_weighing;
    lv_obj_t *prep_content;
    lv_obj_t *prep_content_title;
    lv_obj_t *prep_placeholder;
    lv_obj_t *prep_guide;
    lv_obj_t *workpage_app_title;
    lv_obj_t *workpage_status;
    lv_obj_t *workpage_stop;
    lv_obj_t *workpage_stop_label;
    lv_obj_t *workpage_tab_ridge;
    lv_obj_t *workpage_tab_ridge_label;
    lv_obj_t *workpage_tab_line_ridge;
    lv_obj_t *workpage_tab_prescription;
    lv_obj_t *workpage_tab_prescription_label;
    lv_obj_t *workpage_tab_line_prescription;
    lv_obj_t *workpage_tab_preparation;
    lv_obj_t *workpage_tab_preparation_label;
    lv_obj_t *workpage_tab_line_preparation;
    lv_obj_t *workpage_tab_work;
    lv_obj_t *workpage_tab_work_label;
    lv_obj_t *workpage_tab_line_work;
    lv_obj_t *workpage_tab_weighing;
    lv_obj_t *workpage_tab_weighing_label;
    lv_obj_t *workpage_tab_line_weighing;
    lv_obj_t *workpage_content;
    lv_obj_t *workpage_content_title;
    lv_obj_t *workpage_placeholder;
    lv_obj_t *workpage_guide;
    lv_obj_t *weigh_app_title;
    lv_obj_t *weigh_status;
    lv_obj_t *weigh_stop;
    lv_obj_t *weigh_stop_label;
    lv_obj_t *weigh_tab_ridge;
    lv_obj_t *weigh_tab_ridge_label;
    lv_obj_t *weigh_tab_line_ridge;
    lv_obj_t *weigh_tab_prescription;
    lv_obj_t *weigh_tab_prescription_label;
    lv_obj_t *weigh_tab_line_prescription;
    lv_obj_t *weigh_tab_preparation;
    lv_obj_t *weigh_tab_preparation_label;
    lv_obj_t *weigh_tab_line_preparation;
    lv_obj_t *weigh_tab_work;
    lv_obj_t *weigh_tab_work_label;
    lv_obj_t *weigh_tab_line_work;
    lv_obj_t *weigh_tab_weighing;
    lv_obj_t *weigh_tab_weighing_label;
    lv_obj_t *weigh_tab_line_weighing;
    lv_obj_t *weigh_content;
    lv_obj_t *weigh_content_title;
    lv_obj_t *weigh_placeholder;
    lv_obj_t *weigh_guide;
    lv_obj_t *numeric_input_card;
    lv_obj_t *numeric_input_title_label;
    lv_obj_t *numeric_input_editor;
    lv_obj_t *numeric_input_unit_label;
    lv_obj_t *numeric_input_keypad;
    lv_obj_t *numeric_input_cancel_button;
    lv_obj_t *numeric_input_cancel_button_label;
    lv_obj_t *numeric_input_done_button;
    lv_obj_t *numeric_input_done_button_label;
    lv_obj_t *numeric_input_help_label;
} objects_t;

extern objects_t objects;

void create_screen_startup_animation();
void tick_screen_startup_animation();

void create_screen_main();
void tick_screen_main();

void create_screen_ridge_setup();
void tick_screen_ridge_setup();

void create_screen_preparation();
void tick_screen_preparation();

void create_screen_work();
void tick_screen_work();

void create_screen_weighing_test();
void tick_screen_weighing_test();

void create_screen_numeric_input();
void tick_screen_numeric_input();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/