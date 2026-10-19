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
    SCREEN_ID_DASHBOARD = 2,
    SCREEN_ID_SYSTEM_SETTING = 3,
    SCREEN_ID_RIDGE_SETUP = 4,
    SCREEN_ID_PREPARATION = 5,
    SCREEN_ID_WORK = 6,
    SCREEN_ID_WEIGHING_TEST = 7,
    SCREEN_ID_NUMERIC_INPUT = 8,
    _SCREEN_ID_LAST = 8
};

typedef struct _objects_t {
    lv_obj_t *startup_animation;
    lv_obj_t *dashboard;
    lv_obj_t *system_setting;
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
    lv_obj_t *dashboard_header;
    lv_obj_t *dashboard_brand;
    lv_obj_t *dashboard_title;
    lv_obj_t *dashboard_operation_label;
    lv_obj_t *operation_state_value;
    lv_obj_t *dashboard_can_label;
    lv_obj_t *can_online_value;
    lv_obj_t *dashboard_stop_button;
    lv_obj_t *dashboard_stop_label;
    lv_obj_t *dashboard_motor_section_hint;
    lv_obj_t *motor_1_card;
    lv_obj_t *motor_1_number;
    lv_obj_t *motor_1_name;
    lv_obj_t *motor_1_state_value;
    lv_obj_t *motor_1_speed_label;
    lv_obj_t *motor_1_target_label;
    lv_obj_t *motor_1_speed_value;
    lv_obj_t *motor_1_speed_unit;
    lv_obj_t *motor_1_target_value;
    lv_obj_t *motor_1_target_unit;
    lv_obj_t *motor_1_progress_track;
    lv_obj_t *motor_1_progress_fill;
    lv_obj_t *motor_1_load_value;
    lv_obj_t *motor_2_card;
    lv_obj_t *motor_2_number;
    lv_obj_t *motor_2_name;
    lv_obj_t *motor_2_state_value;
    lv_obj_t *motor_2_speed_label;
    lv_obj_t *motor_2_target_label;
    lv_obj_t *motor_2_speed_value;
    lv_obj_t *motor_2_speed_unit;
    lv_obj_t *motor_2_target_value;
    lv_obj_t *motor_2_target_unit;
    lv_obj_t *motor_2_progress_track;
    lv_obj_t *motor_2_progress_fill;
    lv_obj_t *motor_2_load_value;
    lv_obj_t *motor_3_card;
    lv_obj_t *motor_3_number;
    lv_obj_t *motor_3_name;
    lv_obj_t *motor_3_state_value;
    lv_obj_t *motor_3_speed_label;
    lv_obj_t *motor_3_target_label;
    lv_obj_t *motor_3_speed_value;
    lv_obj_t *motor_3_speed_unit;
    lv_obj_t *motor_3_target_value;
    lv_obj_t *motor_3_target_unit;
    lv_obj_t *motor_3_progress_track;
    lv_obj_t *motor_3_progress_fill;
    lv_obj_t *motor_3_load_value;
    lv_obj_t *motor_4_card;
    lv_obj_t *motor_4_number;
    lv_obj_t *motor_4_name;
    lv_obj_t *motor_4_state_value;
    lv_obj_t *motor_4_speed_label;
    lv_obj_t *motor_4_target_label;
    lv_obj_t *motor_4_speed_value;
    lv_obj_t *motor_4_speed_unit;
    lv_obj_t *motor_4_target_value;
    lv_obj_t *motor_4_target_unit;
    lv_obj_t *motor_4_progress_track;
    lv_obj_t *motor_4_progress_fill;
    lv_obj_t *motor_4_load_value;
    lv_obj_t *vehicle_speed_card;
    lv_obj_t *vehicle_speed_title;
    lv_obj_t *vehicle_speed_divider;
    lv_obj_t *vehicle_speed_label;
    lv_obj_t *vehicle_speed_value;
    lv_obj_t *vehicle_speed_unit;
    lv_obj_t *vehicle_target_speed_label;
    lv_obj_t *vehicle_target_speed_value;
    lv_obj_t *vehicle_speed_progress_track;
    lv_obj_t *vehicle_speed_progress_fill;
    lv_obj_t *vehicle_speed_signal_label;
    lv_obj_t *vehicle_speed_signal_value;
    lv_obj_t *dashboard_system_card;
    lv_obj_t *dashboard_system_title;
    lv_obj_t *work_mode_label;
    lv_obj_t *work_mode_value;
    lv_obj_t *recipe_name_label;
    lv_obj_t *recipe_name_value;
    lv_obj_t *dashboard_alert_card;
    lv_obj_t *dashboard_alert_title;
    lv_obj_t *dashboard_alert_severity;
    lv_obj_t *dashboard_alert_value;
    lv_obj_t *dashboard_setting_button;
    lv_obj_t *dashboard_setting_caption;
    lv_obj_t *dashboard_setting_label;
    lv_obj_t *dashboard_setting_description;
    lv_obj_t *dashboard_setting_arrow;
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
    lv_obj_t *main_right_title;
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
    lv_obj_t *fertilizer_temp_save;
    lv_obj_t *fertilizer_temp_save_label;
    lv_obj_t *fertilizer_save_settings;
    lv_obj_t *fertilizer_save_settings_label;
    lv_obj_t *main_stop_1;
    lv_obj_t *main_stop_label_1;
    lv_obj_t *ridge_divider_1;
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
    lv_obj_t *ridge_left_title;
    lv_obj_t *ridge_divider;
    lv_obj_t *ridge_ridge_label;
    lv_obj_t *ridge_ridge_value;
    lv_obj_t *ridge_count_label;
    lv_obj_t *ridge_recipe_ridge_count_input;
    lv_obj_t *ridge_count_unit;
    lv_obj_t *ridge_width_label;
    lv_obj_t *ridge_recipe_ridge_width_input;
    lv_obj_t *ridge_width_unit;
    lv_obj_t *ridge_length_label;
    lv_obj_t *ridge_recipe_actual_length_input;
    lv_obj_t *ridge_length_unit;
    lv_obj_t *ridge_area_only;
    lv_obj_t *ridge_area_only_label;
    lv_obj_t *ridge_area_furrow;
    lv_obj_t *ridge_area_furrow_label;
    lv_obj_t *ridge_all;
    lv_obj_t *ridge_all_label;
    lv_obj_t *ridge_split;
    lv_obj_t *ridge_split_label;
    lv_obj_t *ridge_apply_length;
    lv_obj_t *ridge_apply_length_label;
    lv_obj_t *ridge_north_label;
    lv_obj_t *ridge_north_value;
    lv_obj_t *ridge_south_label;
    lv_obj_t *ridge_south_value;
    lv_obj_t *ridge_temp_save;
    lv_obj_t *ridge_temp_save_label;
    lv_obj_t *ridge_save_settings;
    lv_obj_t *ridge_save_settings_label;
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

void create_screen_dashboard();
void tick_screen_dashboard();

void create_screen_system_setting();
void tick_screen_system_setting();

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