#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include <string.h>

objects_t objects;

static const char *screen_names[] = { "StartupAnimation", "Dashboard", "MotorManual", "SystemSetting", "RidgeSetup", "Preparation", "Work", "WeighingTest", "NumericInput" };
static const char *object_names[] = { "startup_animation", "dashboard", "motor_manual", "system_setting", "ridge_setup", "preparation", "work", "weighing_test", "numeric_input", "startup_top_accent", "startup_brand_label", "startup_subtitle_label", "startup_hopper_backdrop", "startup_hopper_image", "startup_particle_1", "startup_particle_2", "startup_particle_3", "startup_particle_4", "startup_particle_5", "startup_particle_6", "startup_particle_7", "startup_particle_8", "startup_particle_9", "startup_percent_0", "startup_percent_25", "startup_percent_50", "startup_percent_75", "startup_percent_100", "startup_dispensed_label", "startup_progress_track", "startup_progress_fill", "startup_status_label", "startup_module_label", "startup_footer_label", "dashboard_header", "dashboard_brand", "dashboard_title", "dashboard_operation_label", "operation_state_value", "dashboard_can_label", "can_online_value", "dashboard_stop_button", "dashboard_stop_label", "dashboard_motor_section_hint", "motor_1_card", "motor_1_number", "motor_1_name", "motor_1_state_value", "motor_1_speed_label", "motor_1_target_label", "motor_1_speed_value", "motor_1_speed_unit", "motor_1_target_value", "motor_1_target_unit", "motor_1_progress_track", "motor_1_progress_fill", "motor_1_load_value", "motor_2_card", "motor_2_number", "motor_2_name", "motor_2_state_value", "motor_2_speed_label", "motor_2_target_label", "motor_2_speed_value", "motor_2_speed_unit", "motor_2_target_value", "motor_2_target_unit", "motor_2_progress_track", "motor_2_progress_fill", "motor_2_load_value", "motor_3_card", "motor_3_number", "motor_3_name", "motor_3_state_value", "motor_3_speed_label", "motor_3_target_label", "motor_3_speed_value", "motor_3_speed_unit", "motor_3_target_value", "motor_3_target_unit", "motor_3_progress_track", "motor_3_progress_fill", "motor_3_load_value", "motor_4_card", "motor_4_number", "motor_4_name", "motor_4_state_value", "motor_4_speed_label", "motor_4_target_label", "motor_4_speed_value", "motor_4_speed_unit", "motor_4_target_value", "motor_4_target_unit", "motor_4_progress_track", "motor_4_progress_fill", "motor_4_load_value", "vehicle_speed_card", "vehicle_speed_title", "vehicle_speed_divider", "vehicle_speed_label", "vehicle_speed_value", "vehicle_speed_unit", "vehicle_target_speed_label", "vehicle_target_speed_value", "vehicle_speed_progress_track", "vehicle_speed_progress_fill", "vehicle_speed_signal_label", "vehicle_speed_signal_value", "dashboard_system_card", "dashboard_system_title", "work_mode_label", "work_mode_value", "recipe_name_label", "recipe_name_value", "dashboard_alert_card", "dashboard_alert_title", "dashboard_alert_severity", "dashboard_alert_value", "dashboard_setting_button", "dashboard_setting_caption", "dashboard_setting_label", "dashboard_setting_description", "dashboard_setting_arrow", "motor_manual_backdrop", "motor_manual_panel", "motor_manual_title", "motor_manual_selected_value", "motor_manual_close_button", "motor_manual_close_label", "motor_manual_rpm_caption", "motor_manual_rpm_value", "motor_manual_rpm_unit", "motor_manual_rpm_slider", "motor_manual_range_hint", "motor_manual_direction_value", "motor_manual_operation_value", "motor_manual_forward_button", "motor_manual_forward_label", "motor_manual_reverse_button", "motor_manual_reverse_label", "motor_manual_run_button", "motor_manual_run_label", "motor_manual_stop_button", "motor_manual_stop_label", "motor_manual_run_lamp", "motor_manual_run_lamp_label", "motor_manual_stop_lamp", "motor_manual_stop_lamp_label", "motor_manual_safety_hint", "main_status", "main_stop", "main_stop_label", "main_tab_ridge", "main_tab_ridge_label", "main_tab_line_ridge", "main_tab_prescription", "main_tab_prescription_label", "main_tab_line_prescription", "main_tab_preparation", "main_tab_preparation_label", "main_tab_line_preparation", "main_tab_work", "main_tab_work_label", "main_tab_line_work", "main_tab_weighing", "main_tab_weighing_label", "main_tab_line_weighing", "main_content", "main_right_title", "main_nitrogen", "main_nitrogen_label", "main_direct", "main_direct_label", "main_nitrogen_amount_label", "recipe_nitrogen_rate_input", "main_nitrogen_unit", "main_upper_ratio_label", "main_upper_ratio", "main_lower_ratio_label", "main_lower_ratio", "main_upper_product_label", "main_upper_product", "main_lower_product_label", "main_lower_product", "fertilizer_temp_save", "fertilizer_temp_save_label", "fertilizer_save_settings", "fertilizer_save_settings_label", "main_stop_1", "main_stop_label_1", "ridge_divider_1", "ridge_app_title", "ridge_status", "ridge_stop", "ridge_stop_label", "ridge_tab_ridge", "ridge_tab_ridge_label", "ridge_tab_line_ridge", "ridge_tab_prescription", "ridge_tab_prescription_label", "ridge_tab_line_prescription", "ridge_tab_preparation", "ridge_tab_preparation_label", "ridge_tab_line_preparation", "ridge_tab_work", "ridge_tab_work_label", "ridge_tab_line_work", "ridge_tab_weighing", "ridge_tab_weighing_label", "ridge_tab_line_weighing", "ridge_content", "ridge_left_title", "ridge_divider", "ridge_ridge_label", "ridge_ridge_value", "ridge_count_label", "ridge_recipe_ridge_count_input", "ridge_count_unit", "ridge_width_label", "ridge_recipe_ridge_width_input", "ridge_width_unit", "ridge_length_label", "ridge_recipe_actual_length_input", "ridge_length_unit", "ridge_area_only", "ridge_area_only_label", "ridge_area_furrow", "ridge_area_furrow_label", "ridge_all", "ridge_all_label", "ridge_split", "ridge_split_label", "ridge_apply_length", "ridge_apply_length_label", "ridge_north_label", "ridge_north_value", "ridge_south_label", "ridge_south_value", "ridge_temp_save", "ridge_temp_save_label", "ridge_save_settings", "ridge_save_settings_label", "prep_app_title", "prep_status", "prep_stop", "prep_stop_label", "prep_tab_ridge", "prep_tab_ridge_label", "prep_tab_line_ridge", "prep_tab_prescription", "prep_tab_prescription_label", "prep_tab_line_prescription", "prep_tab_preparation", "prep_tab_preparation_label", "prep_tab_line_preparation", "prep_tab_work", "prep_tab_work_label", "prep_tab_line_work", "prep_tab_weighing", "prep_tab_weighing_label", "prep_tab_line_weighing", "prep_content", "prep_content_title", "prep_placeholder", "prep_guide", "workpage_app_title", "workpage_status", "workpage_stop", "workpage_stop_label", "workpage_tab_ridge", "workpage_tab_ridge_label", "workpage_tab_line_ridge", "workpage_tab_prescription", "workpage_tab_prescription_label", "workpage_tab_line_prescription", "workpage_tab_preparation", "workpage_tab_preparation_label", "workpage_tab_line_preparation", "workpage_tab_work", "workpage_tab_work_label", "workpage_tab_line_work", "workpage_tab_weighing", "workpage_tab_weighing_label", "workpage_tab_line_weighing", "workpage_content", "workpage_content_title", "workpage_placeholder", "workpage_guide", "weigh_app_title", "weigh_status", "weigh_stop", "weigh_stop_label", "weigh_tab_ridge", "weigh_tab_ridge_label", "weigh_tab_line_ridge", "weigh_tab_prescription", "weigh_tab_prescription_label", "weigh_tab_line_prescription", "weigh_tab_preparation", "weigh_tab_preparation_label", "weigh_tab_line_preparation", "weigh_tab_work", "weigh_tab_work_label", "weigh_tab_line_work", "weigh_tab_weighing", "weigh_tab_weighing_label", "weigh_tab_line_weighing", "weigh_content", "weigh_content_title", "weigh_placeholder", "weigh_guide", "numeric_input_card", "numeric_input_title_label", "numeric_input_editor", "numeric_input_unit_label", "numeric_input_keypad", "numeric_input_cancel_button", "numeric_input_cancel_button_label", "numeric_input_done_button", "numeric_input_done_button_label", "numeric_input_help_label" };

//
// Event handlers
//

lv_obj_t *tick_value_change_obj;

static void event_handler_cb_dashboard_motor_1_card(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_dashboard_motor_2_card(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 24, 0, e);
    }
}

static void event_handler_cb_dashboard_motor_3_card(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 37, 0, e);
    }
}

static void event_handler_cb_dashboard_motor_4_card(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 50, 0, e);
    }
}

static void event_handler_cb_dashboard_dashboard_setting_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 85, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_close_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_rpm_slider(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            int32_t value = lv_slider_get_value(ta);
            assignIntegerProperty(flowState, 10, 3, value, "Failed to assign Value in Slider widget");
        }
    }
    if (event == LV_EVENT_VALUE_CHANGED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 10, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_forward_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 14, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_reverse_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 16, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_run_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 18, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_stop_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 20, 0, e);
    }
}

static void event_handler_cb_motor_manual_motor_manual_run_lamp(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            assignBooleanProperty(flowState, 22, 3, value, "Failed to assign Checked state");
        }
    }
}

static void event_handler_cb_motor_manual_motor_manual_stop_lamp(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            bool value = lv_obj_has_state(ta, LV_STATE_CHECKED);
            assignBooleanProperty(flowState, 24, 3, value, "Failed to assign Checked state");
        }
    }
}

static void event_handler_cb_system_setting_main_tab_ridge(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 4, 0, e);
    }
}

static void event_handler_cb_system_setting_main_tab_prescription(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 7, 0, e);
    }
}

static void event_handler_cb_system_setting_main_tab_preparation(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 10, 0, e);
    }
}

static void event_handler_cb_system_setting_main_tab_work(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 13, 0, e);
    }
}

static void event_handler_cb_system_setting_main_tab_weighing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 16, 0, e);
    }
}

static void event_handler_cb_system_setting_recipe_nitrogen_rate_input(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 26, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)4;
        flowPropagateValueLVGLEvent(flowState, 26, 0, e);
    }
}

static void event_handler_cb_system_setting_main_stop_1(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 40, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_tab_ridge(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_tab_prescription(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_tab_preparation(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_tab_work(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 14, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_tab_weighing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 17, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_recipe_ridge_count_input(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 26, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)1;
        flowPropagateValueLVGLEvent(flowState, 26, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_recipe_ridge_width_input(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 29, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)2;
        flowPropagateValueLVGLEvent(flowState, 29, 0, e);
    }
}

static void event_handler_cb_ridge_setup_ridge_recipe_actual_length_input(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 32, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)3;
        flowPropagateValueLVGLEvent(flowState, 32, 0, e);
    }
}

static void event_handler_cb_preparation_prep_tab_ridge(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_preparation_prep_tab_prescription(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_preparation_prep_tab_preparation(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_preparation_prep_tab_work(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 14, 0, e);
    }
}

static void event_handler_cb_preparation_prep_tab_weighing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 17, 0, e);
    }
}

static void event_handler_cb_work_workpage_tab_ridge(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_work_workpage_tab_prescription(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_work_workpage_tab_preparation(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_work_workpage_tab_work(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 14, 0, e);
    }
}

static void event_handler_cb_work_workpage_tab_weighing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 17, 0, e);
    }
}

static void event_handler_cb_weighing_test_weigh_tab_ridge(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 5, 0, e);
    }
}

static void event_handler_cb_weighing_test_weigh_tab_prescription(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

static void event_handler_cb_weighing_test_weigh_tab_preparation(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 11, 0, e);
    }
}

static void event_handler_cb_weighing_test_weigh_tab_work(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 14, 0, e);
    }
}

static void event_handler_cb_weighing_test_weigh_tab_weighing(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 17, 0, e);
    }
}

static void event_handler_cb_numeric_input_numeric_input_editor(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *ta = lv_event_get_target_obj(e);
        if (tick_value_change_obj != ta) {
            const char *value = lv_textarea_get_text(ta);
            assignStringProperty(flowState, 3, 3, value, "Failed to assign Text in Textarea widget");
        }
    }
}

static void event_handler_cb_numeric_input_numeric_input_cancel_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 6, 0, e);
    }
}

static void event_handler_cb_numeric_input_numeric_input_done_button(lv_event_t *e) {
    lv_event_code_t event = lv_event_get_code(e);
    void *flowState = lv_event_get_user_data(e);
    (void)flowState;
    
    if (event == LV_EVENT_PRESSED) {
        e->user_data = (void *)0;
        flowPropagateValueLVGLEvent(flowState, 8, 0, e);
    }
}

//
// Screens
//

void create_screen_startup_animation() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.startup_animation = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0d1115), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // startup_top_accent
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_top_accent = obj;
            lv_obj_set_pos(obj, 58, 55);
            lv_obj_set_size(obj, 205, 5);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_brand_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_brand_label = obj;
            lv_obj_set_pos(obj, 40, 88);
            lv_obj_set_size(obj, 282, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf5f7f9), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DAJILOG");
        }
        {
            // startup_subtitle_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_subtitle_label = obj;
            lv_obj_set_pos(obj, 38, 144);
            lv_obj_set_size(obj, 287, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8f99a3), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "INTELLIGENT FERTILIZER\nCONTROL");
        }
        {
            // startup_hopper_backdrop
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_hopper_backdrop = obj;
            lv_obj_set_pos(obj, 348, 78);
            lv_obj_set_size(obj, 320, 320);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1c242a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x34404b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_hopper_image
            lv_obj_t *obj = lv_image_create(parent_obj);
            objects.startup_hopper_image = obj;
            lv_obj_set_pos(obj, 458, 140);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_hopper_icon_100);
            lv_image_set_pivot(obj, 50, 50);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_ADV_HITTEST|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
        }
        {
            // startup_particle_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_1 = obj;
            lv_obj_set_pos(obj, 501, 275);
            lv_obj_set_size(obj, 8, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_2
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_2 = obj;
            lv_obj_set_pos(obj, 483, 288);
            lv_obj_set_size(obj, 7, 7);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_3
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_3 = obj;
            lv_obj_set_pos(obj, 520, 289);
            lv_obj_set_size(obj, 7, 7);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_4
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_4 = obj;
            lv_obj_set_pos(obj, 468, 305);
            lv_obj_set_size(obj, 6, 6);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_5
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_5 = obj;
            lv_obj_set_pos(obj, 539, 305);
            lv_obj_set_size(obj, 6, 6);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_6
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_6 = obj;
            lv_obj_set_pos(obj, 494, 313);
            lv_obj_set_size(obj, 7, 7);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_7
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_7 = obj;
            lv_obj_set_pos(obj, 521, 320);
            lv_obj_set_size(obj, 6, 6);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_8
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_8 = obj;
            lv_obj_set_pos(obj, 508, 333);
            lv_obj_set_size(obj, 5, 5);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_particle_9
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_particle_9 = obj;
            lv_obj_set_pos(obj, 549, 328);
            lv_obj_set_size(obj, 5, 5);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_percent_0
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_percent_0 = obj;
            lv_obj_set_pos(obj, 727, 140);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "0%");
        }
        {
            // startup_percent_25
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_percent_25 = obj;
            lv_obj_set_pos(obj, 727, 140);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "25%");
        }
        {
            // startup_percent_50
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_percent_50 = obj;
            lv_obj_set_pos(obj, 727, 140);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "50%");
        }
        {
            // startup_percent_75
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_percent_75 = obj;
            lv_obj_set_pos(obj, 727, 140);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "75%");
        }
        {
            // startup_percent_100
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_percent_100 = obj;
            lv_obj_set_pos(obj, 727, 140);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "100%");
        }
        {
            // startup_dispensed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_dispensed_label = obj;
            lv_obj_set_pos(obj, 727, 198);
            lv_obj_set_size(obj, 243, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x87929c), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DISPENSED");
        }
        {
            // startup_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_progress_track = obj;
            lv_obj_set_pos(obj, 735, 253);
            lv_obj_set_size(obj, 228, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x303840), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x303840), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.startup_progress_fill = obj;
            lv_obj_set_pos(obj, 735, 253);
            lv_obj_set_size(obj, 1, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe8ba3f), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // startup_status_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_status_label = obj;
            lv_obj_set_pos(obj, 700, 315);
            lv_obj_set_size(obj, 294, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf0f3f6), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM STARTING");
        }
        {
            // startup_module_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_module_label = obj;
            lv_obj_set_pos(obj, 700, 365);
            lv_obj_set_size(obj, 294, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x78838d), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "LOADING DISPENSING\nMODULE");
        }
        {
            // startup_footer_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.startup_footer_label = obj;
            lv_obj_set_pos(obj, 38, 531);
            lv_obj_set_size(obj, 948, LV_SIZE_CONTENT);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x5c6872), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DAJILOG LABS  /  SYSTEM BOOT");
        }
    }
    
    tick_screen_startup_animation();
}

void tick_screen_startup_animation() {
    void *flowState = getFlowState(0, 0);
    (void)flowState;
}

void create_screen_dashboard() {
    // dashboard_screen
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.dashboard = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x091018), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // dashboard_header
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.dashboard_header = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 1024, 64);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dashboard_brand
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_brand = obj;
            lv_obj_set_pos(obj, 24, 10);
            lv_obj_set_size(obj, 261, 32);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "AGMO  |  농진원 시비 제어");
        }
        {
            // dashboard_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_title = obj;
            lv_obj_set_pos(obj, 356, 17);
            lv_obj_set_size(obj, 244, 44);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DASHBOARD");
        }
        {
            // dashboard_operation_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_operation_label = obj;
            lv_obj_set_pos(obj, 655, 19);
            lv_obj_set_size(obj, 52, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "운전");
        }
        {
            // operation_state_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.operation_state_value = obj;
            lv_obj_set_pos(obj, 707, 18);
            lv_obj_set_size(obj, 64, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // dashboard_can_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_can_label = obj;
            lv_obj_set_pos(obj, 781, 19);
            lv_obj_set_size(obj, 46, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "CAN");
        }
        {
            // can_online_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.can_online_value = obj;
            lv_obj_set_pos(obj, 827, 18);
            lv_obj_set_size(obj, 66, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // dashboard_stop_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.dashboard_stop_button = obj;
            lv_obj_set_pos(obj, 905, 10);
            lv_obj_set_size(obj, 103, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf04444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xba2424), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // dashboard_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dashboard_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 11);
                    lv_obj_set_size(obj, 103, 22);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // dashboard_motor_section_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_motor_section_hint = obj;
            lv_obj_set_pos(obj, 65, 39);
            lv_obj_set_size(obj, 180, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "4채널 실시간 운전 정보");
        }
        {
            // motor_1_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_1_card = obj;
            lv_obj_set_pos(obj, 24, 78);
            lv_obj_set_size(obj, 316, 164);
            lv_obj_add_event_cb(obj, event_handler_cb_dashboard_motor_1_card, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_1_number
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_number = obj;
            lv_obj_set_pos(obj, 40, 90);
            lv_obj_set_size(obj, 42, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "M1");
        }
        {
            // motor_1_name
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_name = obj;
            lv_obj_set_pos(obj, 82, 88);
            lv_obj_set_size(obj, 116, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "전층 A");
        }
        {
            // motor_1_state_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_state_value = obj;
            lv_obj_set_pos(obj, 246, 88);
            lv_obj_set_size(obj, 78, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_1_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_speed_label = obj;
            lv_obj_set_pos(obj, 40, 132);
            lv_obj_set_size(obj, 96, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 속도");
        }
        {
            // motor_1_target_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_target_label = obj;
            lv_obj_set_pos(obj, 200, 132);
            lv_obj_set_size(obj, 72, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "목표량");
        }
        {
            // motor_1_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_speed_value = obj;
            lv_obj_set_pos(obj, 40, 154);
            lv_obj_set_size(obj, 96, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_1_speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_speed_unit = obj;
            lv_obj_set_pos(obj, 138, 161);
            lv_obj_set_size(obj, 48, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "rpm");
        }
        {
            // motor_1_target_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_target_value = obj;
            lv_obj_set_pos(obj, 200, 154);
            lv_obj_set_size(obj, 72, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_1_target_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_target_unit = obj;
            lv_obj_set_pos(obj, 276, 161);
            lv_obj_set_size(obj, 58, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "kg/min");
        }
        {
            // motor_1_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_1_progress_track = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, 284, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_1_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_1_progress_fill = obj;
            lv_obj_set_pos(obj, 40, 208);
            lv_obj_set_size(obj, 204, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_1_load_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_1_load_value = obj;
            lv_obj_set_pos(obj, 269, 189);
            lv_obj_set_size(obj, 55, 18);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_2_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_2_card = obj;
            lv_obj_set_pos(obj, 356, 78);
            lv_obj_set_size(obj, 316, 164);
            lv_obj_add_event_cb(obj, event_handler_cb_dashboard_motor_2_card, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_2_number
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_number = obj;
            lv_obj_set_pos(obj, 372, 90);
            lv_obj_set_size(obj, 42, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "M2");
        }
        {
            // motor_2_name
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_name = obj;
            lv_obj_set_pos(obj, 414, 88);
            lv_obj_set_size(obj, 116, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "전층 B");
        }
        {
            // motor_2_state_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_state_value = obj;
            lv_obj_set_pos(obj, 578, 88);
            lv_obj_set_size(obj, 78, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_2_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_speed_label = obj;
            lv_obj_set_pos(obj, 372, 132);
            lv_obj_set_size(obj, 96, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 속도");
        }
        {
            // motor_2_target_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_target_label = obj;
            lv_obj_set_pos(obj, 532, 132);
            lv_obj_set_size(obj, 72, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "목표량");
        }
        {
            // motor_2_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_speed_value = obj;
            lv_obj_set_pos(obj, 372, 154);
            lv_obj_set_size(obj, 96, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_2_speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_speed_unit = obj;
            lv_obj_set_pos(obj, 470, 161);
            lv_obj_set_size(obj, 48, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "rpm");
        }
        {
            // motor_2_target_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_target_value = obj;
            lv_obj_set_pos(obj, 532, 154);
            lv_obj_set_size(obj, 72, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_2_target_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_target_unit = obj;
            lv_obj_set_pos(obj, 608, 161);
            lv_obj_set_size(obj, 58, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "kg/min");
        }
        {
            // motor_2_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_2_progress_track = obj;
            lv_obj_set_pos(obj, 372, 208);
            lv_obj_set_size(obj, 284, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_2_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_2_progress_fill = obj;
            lv_obj_set_pos(obj, 372, 208);
            lv_obj_set_size(obj, 2, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_2_load_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_2_load_value = obj;
            lv_obj_set_pos(obj, 601, 189);
            lv_obj_set_size(obj, 55, 18);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_3_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_3_card = obj;
            lv_obj_set_pos(obj, 24, 258);
            lv_obj_set_size(obj, 316, 164);
            lv_obj_add_event_cb(obj, event_handler_cb_dashboard_motor_3_card, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_3_number
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_number = obj;
            lv_obj_set_pos(obj, 40, 270);
            lv_obj_set_size(obj, 42, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "M3");
        }
        {
            // motor_3_name
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_name = obj;
            lv_obj_set_pos(obj, 82, 268);
            lv_obj_set_size(obj, 116, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "지중 C");
        }
        {
            // motor_3_state_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_state_value = obj;
            lv_obj_set_pos(obj, 246, 268);
            lv_obj_set_size(obj, 78, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_3_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_speed_label = obj;
            lv_obj_set_pos(obj, 40, 312);
            lv_obj_set_size(obj, 96, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 속도");
        }
        {
            // motor_3_target_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_target_label = obj;
            lv_obj_set_pos(obj, 200, 312);
            lv_obj_set_size(obj, 72, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "목표량");
        }
        {
            // motor_3_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_speed_value = obj;
            lv_obj_set_pos(obj, 40, 334);
            lv_obj_set_size(obj, 96, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_3_speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_speed_unit = obj;
            lv_obj_set_pos(obj, 138, 341);
            lv_obj_set_size(obj, 48, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "rpm");
        }
        {
            // motor_3_target_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_target_value = obj;
            lv_obj_set_pos(obj, 200, 334);
            lv_obj_set_size(obj, 72, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_3_target_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_target_unit = obj;
            lv_obj_set_pos(obj, 276, 341);
            lv_obj_set_size(obj, 58, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "kg/min");
        }
        {
            // motor_3_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_3_progress_track = obj;
            lv_obj_set_pos(obj, 40, 388);
            lv_obj_set_size(obj, 284, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_3_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_3_progress_fill = obj;
            lv_obj_set_pos(obj, 40, 388);
            lv_obj_set_size(obj, 181, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_3_load_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_3_load_value = obj;
            lv_obj_set_pos(obj, 269, 369);
            lv_obj_set_size(obj, 55, 18);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_4_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_4_card = obj;
            lv_obj_set_pos(obj, 356, 258);
            lv_obj_set_size(obj, 316, 164);
            lv_obj_add_event_cb(obj, event_handler_cb_dashboard_motor_4_card, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_4_number
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_number = obj;
            lv_obj_set_pos(obj, 372, 270);
            lv_obj_set_size(obj, 42, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "M4");
        }
        {
            // motor_4_name
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_name = obj;
            lv_obj_set_pos(obj, 414, 268);
            lv_obj_set_size(obj, 116, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "지중 D");
        }
        {
            // motor_4_state_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_state_value = obj;
            lv_obj_set_pos(obj, 578, 268);
            lv_obj_set_size(obj, 78, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf04444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_4_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_speed_label = obj;
            lv_obj_set_pos(obj, 372, 312);
            lv_obj_set_size(obj, 96, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 속도");
        }
        {
            // motor_4_target_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_target_label = obj;
            lv_obj_set_pos(obj, 532, 312);
            lv_obj_set_size(obj, 72, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "목표량");
        }
        {
            // motor_4_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_speed_value = obj;
            lv_obj_set_pos(obj, 372, 334);
            lv_obj_set_size(obj, 96, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_4_speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_speed_unit = obj;
            lv_obj_set_pos(obj, 470, 341);
            lv_obj_set_size(obj, 48, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "rpm");
        }
        {
            // motor_4_target_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_target_value = obj;
            lv_obj_set_pos(obj, 532, 334);
            lv_obj_set_size(obj, 72, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_4_target_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_target_unit = obj;
            lv_obj_set_pos(obj, 608, 341);
            lv_obj_set_size(obj, 58, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "kg/min");
        }
        {
            // motor_4_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_4_progress_track = obj;
            lv_obj_set_pos(obj, 372, 388);
            lv_obj_set_size(obj, 284, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_4_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_4_progress_fill = obj;
            lv_obj_set_pos(obj, 372, 388);
            lv_obj_set_size(obj, 51, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf04444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf04444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_4_load_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_4_load_value = obj;
            lv_obj_set_pos(obj, 601, 369);
            lv_obj_set_size(obj, 55, 18);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf04444), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // vehicle_speed_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.vehicle_speed_card = obj;
            lv_obj_set_pos(obj, 688, 78);
            lv_obj_set_size(obj, 312, 218);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // vehicle_speed_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_title = obj;
            lv_obj_set_pos(obj, 704, 88);
            lv_obj_set_size(obj, 210, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "작업기 운행 속도");
        }
        {
            // vehicle_speed_divider
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.vehicle_speed_divider = obj;
            lv_obj_set_pos(obj, 704, 125);
            lv_obj_set_size(obj, 280, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // vehicle_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_label = obj;
            lv_obj_set_pos(obj, 704, 134);
            lv_obj_set_size(obj, 100, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 속도");
        }
        {
            // vehicle_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_value = obj;
            lv_obj_set_pos(obj, 704, 154);
            lv_obj_set_size(obj, 150, 64);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // vehicle_speed_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_unit = obj;
            lv_obj_set_pos(obj, 862, 176);
            lv_obj_set_size(obj, 58, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "km/h");
        }
        {
            // vehicle_target_speed_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_target_speed_label = obj;
            lv_obj_set_pos(obj, 704, 223);
            lv_obj_set_size(obj, 80, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "목표 속도");
        }
        {
            // vehicle_target_speed_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_target_speed_value = obj;
            lv_obj_set_pos(obj, 810, 223);
            lv_obj_set_size(obj, 174, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // vehicle_speed_progress_track
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.vehicle_speed_progress_track = obj;
            lv_obj_set_pos(obj, 704, 258);
            lv_obj_set_size(obj, 280, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x1e2933), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // vehicle_speed_progress_fill
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.vehicle_speed_progress_fill = obj;
            lv_obj_set_pos(obj, 704, 258);
            lv_obj_set_size(obj, 252, 8);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x318ff5), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // vehicle_speed_signal_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_signal_label = obj;
            lv_obj_set_pos(obj, 704, 269);
            lv_obj_set_size(obj, 80, 23);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "속도 신호");
        }
        {
            // vehicle_speed_signal_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.vehicle_speed_signal_value = obj;
            lv_obj_set_pos(obj, 779, 270);
            lv_obj_set_size(obj, 198, 23);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x33d17a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // dashboard_system_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.dashboard_system_card = obj;
            lv_obj_set_pos(obj, 688, 308);
            lv_obj_set_size(obj, 312, 114);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dashboard_system_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_system_title = obj;
            lv_obj_set_pos(obj, 704, 318);
            lv_obj_set_size(obj, 100, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM");
        }
        {
            // work_mode_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.work_mode_label = obj;
            lv_obj_set_pos(obj, 704, 352);
            lv_obj_set_size(obj, 100, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "작업 모드");
        }
        {
            // work_mode_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.work_mode_value = obj;
            lv_obj_set_pos(obj, 810, 352);
            lv_obj_set_size(obj, 174, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // recipe_name_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.recipe_name_label = obj;
            lv_obj_set_pos(obj, 704, 382);
            lv_obj_set_size(obj, 100, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8c9ead), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "현재 처방");
        }
        {
            // recipe_name_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.recipe_name_value = obj;
            lv_obj_set_pos(obj, 810, 382);
            lv_obj_set_size(obj, 174, 24);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // dashboard_alert_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.dashboard_alert_card = obj;
            lv_obj_set_pos(obj, 24, 437);
            lv_obj_set_size(obj, 648, 92);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dashboard_alert_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_alert_title = obj;
            lv_obj_set_pos(obj, 40, 447);
            lv_obj_set_size(obj, 130, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "알림 / 상태");
        }
        {
            // dashboard_alert_severity
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.dashboard_alert_severity = obj;
            lv_obj_set_pos(obj, 40, 478);
            lv_obj_set_size(obj, 4, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // dashboard_alert_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.dashboard_alert_value = obj;
            lv_obj_set_pos(obj, 58, 473);
            lv_obj_set_size(obj, 620, 46);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // dashboard_setting_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.dashboard_setting_button = obj;
            lv_obj_set_pos(obj, 688, 437);
            lv_obj_set_size(obj, 312, 92);
            lv_obj_add_event_cb(obj, event_handler_cb_dashboard_dashboard_setting_button, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd89a16), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // dashboard_setting_caption
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dashboard_setting_caption = obj;
                    lv_obj_set_pos(obj, 66, 10);
                    lv_obj_set_size(obj, 180, 18);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x493b10), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "CONFIGURATION");
                }
                {
                    // dashboard_setting_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dashboard_setting_label = obj;
                    lv_obj_set_pos(obj, 61, 31);
                    lv_obj_set_size(obj, 190, 38);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x191506), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "SETTING");
                }
                {
                    // dashboard_setting_description
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dashboard_setting_description = obj;
                    lv_obj_set_pos(obj, 61, 65);
                    lv_obj_set_size(obj, 190, 22);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x493b10), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "운전 및 장치 설정");
                }
                {
                    // dashboard_setting_arrow
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.dashboard_setting_arrow = obj;
                    lv_obj_set_pos(obj, 235, 24);
                    lv_obj_set_size(obj, 38, 52);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x191506), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, ">");
                }
            }
        }
    }
    
    tick_screen_dashboard();
}

void tick_screen_dashboard() {
    void *flowState = getFlowState(0, 1);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 5, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.operation_state_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.operation_state_value;
            lv_label_set_text(objects.operation_state_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 7, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.can_online_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.can_online_value;
            lv_label_set_text(objects.can_online_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 14, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_1_state_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_1_state_value;
            lv_label_set_text(objects.motor_1_state_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 17, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_1_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_1_speed_value;
            lv_label_set_text(objects.motor_1_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 19, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_1_target_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_1_target_value;
            lv_label_set_text(objects.motor_1_target_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 23, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_1_load_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_1_load_value;
            lv_label_set_text(objects.motor_1_load_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 27, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_2_state_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_2_state_value;
            lv_label_set_text(objects.motor_2_state_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 30, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_2_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_2_speed_value;
            lv_label_set_text(objects.motor_2_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 32, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_2_target_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_2_target_value;
            lv_label_set_text(objects.motor_2_target_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 36, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_2_load_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_2_load_value;
            lv_label_set_text(objects.motor_2_load_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 40, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_3_state_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_3_state_value;
            lv_label_set_text(objects.motor_3_state_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 43, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_3_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_3_speed_value;
            lv_label_set_text(objects.motor_3_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 45, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_3_target_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_3_target_value;
            lv_label_set_text(objects.motor_3_target_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 48, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_3_load_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_3_load_value;
            lv_label_set_text(objects.motor_3_load_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 53, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_4_state_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_4_state_value;
            lv_label_set_text(objects.motor_4_state_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 56, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_4_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_4_speed_value;
            lv_label_set_text(objects.motor_4_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 58, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_4_target_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_4_target_value;
            lv_label_set_text(objects.motor_4_target_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 62, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_4_load_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_4_load_value;
            lv_label_set_text(objects.motor_4_load_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 67, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.vehicle_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.vehicle_speed_value;
            lv_label_set_text(objects.vehicle_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 70, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.vehicle_target_speed_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.vehicle_target_speed_value;
            lv_label_set_text(objects.vehicle_target_speed_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 74, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.vehicle_speed_signal_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.vehicle_speed_signal_value;
            lv_label_set_text(objects.vehicle_speed_signal_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 78, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.work_mode_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.work_mode_value;
            lv_label_set_text(objects.work_mode_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 80, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.recipe_name_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.recipe_name_value;
            lv_label_set_text(objects.recipe_name_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 84, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.dashboard_alert_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.dashboard_alert_value;
            lv_label_set_text(objects.dashboard_alert_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_motor_manual() {
    // motor_manual_screen
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.motor_manual = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x071019), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // motor_manual_backdrop
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_manual_backdrop = obj;
            lv_obj_set_pos(obj, 0, 0);
            lv_obj_set_size(obj, 1024, 600);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x05090e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 235, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_manual_panel
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_manual_panel = obj;
            lv_obj_set_pos(obj, 172, 34);
            lv_obj_set_size(obj, 680, 463);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x344453), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(obj, 180, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(obj, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // motor_manual_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_title = obj;
            lv_obj_set_pos(obj, 212, 52);
            lv_obj_set_size(obj, 430, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf5f7fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "MANUAL MOTOR CONTROL");
        }
        {
            // motor_manual_selected_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_selected_value = obj;
            lv_obj_set_pos(obj, 212, 98);
            lv_obj_set_size(obj, 360, 30);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_manual_close_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.motor_manual_close_button = obj;
            lv_obj_set_pos(obj, 770, 44);
            lv_obj_set_size(obj, 54, 54);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_close_button, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1c2833), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x344453), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // motor_manual_close_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.motor_manual_close_label = obj;
                    lv_obj_set_pos(obj, 0, 9);
                    lv_obj_set_size(obj, 54, 32);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xd9e0e6), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "X");
                }
            }
        }
        {
            // motor_manual_rpm_caption
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_rpm_caption = obj;
            lv_obj_set_pos(obj, 212, 150);
            lv_obj_set_size(obj, 180, 26);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8fa0ae), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "TARGET RPM");
        }
        {
            // motor_manual_rpm_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_rpm_value = obj;
            lv_obj_set_pos(obj, 282, 138);
            lv_obj_set_size(obj, 240, 64);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf5f7fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_manual_rpm_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_rpm_unit = obj;
            lv_obj_set_pos(obj, 539, 162);
            lv_obj_set_size(obj, 80, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8fa0ae), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "rpm");
        }
        {
            // motor_manual_rpm_slider
            lv_obj_t *obj = lv_slider_create(parent_obj);
            objects.motor_manual_rpm_slider = obj;
            lv_obj_set_pos(obj, 222, 218);
            lv_obj_set_size(obj, 580, 34);
            lv_slider_set_range(obj, 0, 30);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_rpm_slider, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x263442), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 12, LV_PART_INDICATOR | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 18, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 7, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 7, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_KNOB | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_KNOB | LV_STATE_DEFAULT);
        }
        {
            // motor_manual_range_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_range_hint = obj;
            lv_obj_set_pos(obj, 222, 262);
            lv_obj_set_size(obj, 580, 26);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x657786), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "0                                      30 RPM");
        }
        {
            // motor_manual_direction_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_direction_value = obj;
            lv_obj_set_pos(obj, 222, 304);
            lv_obj_set_size(obj, 280, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8fa0ae), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_manual_operation_value
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_operation_value = obj;
            lv_obj_set_pos(obj, 522, 304);
            lv_obj_set_size(obj, 280, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xf5f7fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "");
        }
        {
            // motor_manual_forward_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.motor_manual_forward_button = obj;
            lv_obj_set_pos(obj, 212, 348);
            lv_obj_set_size(obj, 135, 82);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_forward_button, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x2677d4), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x195ba7), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // motor_manual_forward_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.motor_manual_forward_label = obj;
                    lv_obj_set_pos(obj, 0, 25);
                    lv_obj_set_size(obj, 135, 32);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "FORWARD");
                }
            }
        }
        {
            // motor_manual_reverse_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.motor_manual_reverse_button = obj;
            lv_obj_set_pos(obj, 367, 348);
            lv_obj_set_size(obj, 135, 82);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_reverse_button, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd17a22), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xa75d16), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // motor_manual_reverse_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.motor_manual_reverse_label = obj;
                    lv_obj_set_pos(obj, 0, 25);
                    lv_obj_set_size(obj, 135, 32);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "REVERSE");
                }
            }
        }
        {
            // motor_manual_run_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.motor_manual_run_button = obj;
            lv_obj_set_pos(obj, 522, 348);
            lv_obj_set_size(obj, 135, 82);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_run_button, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16a34a), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x0d7a35), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // motor_manual_run_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.motor_manual_run_label = obj;
                    lv_obj_set_pos(obj, 0, 25);
                    lv_obj_set_size(obj, 135, 32);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "RUN");
                }
            }
        }
        {
            // motor_manual_stop_button
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.motor_manual_stop_button = obj;
            lv_obj_set_pos(obj, 677, 348);
            lv_obj_set_size(obj, 135, 82);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_stop_button, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_ON_FOCUS|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe33d42), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xb52127), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // motor_manual_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.motor_manual_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 25);
                    lv_obj_set_size(obj, 135, 32);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "STOP");
                }
            }
        }
        {
            // motor_manual_run_lamp
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_manual_run_lamp = obj;
            lv_obj_set_pos(obj, 610, 134);
            lv_obj_set_size(obj, 18, 18);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_run_lamp, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x263442), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 9, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x465664), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x22c55e), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9affb8), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0x22c55e), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_opa(obj, 180, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_width(obj, 8, LV_PART_MAIN | LV_STATE_CHECKED);
        }
        {
            // motor_manual_run_lamp_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_run_lamp_label = obj;
            lv_obj_set_pos(obj, 634, 130);
            lv_obj_set_size(obj, 50, 26);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8fa0ae), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "RUN");
        }
        {
            // motor_manual_stop_lamp
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.motor_manual_stop_lamp = obj;
            lv_obj_set_pos(obj, 690, 134);
            lv_obj_set_size(obj, 18, 18);
            lv_obj_add_event_cb(obj, event_handler_cb_motor_manual_motor_manual_stop_lamp, LV_EVENT_ALL, flowState);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x263442), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 9, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x465664), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xef4444), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xffaaaa), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xef4444), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_opa(obj, 180, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_width(obj, 8, LV_PART_MAIN | LV_STATE_CHECKED);
        }
        {
            // motor_manual_stop_lamp_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_stop_lamp_label = obj;
            lv_obj_set_pos(obj, 714, 130);
            lv_obj_set_size(obj, 54, 26);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8fa0ae), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "STOP");
        }
        {
            // motor_manual_safety_hint
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.motor_manual_safety_hint = obj;
            lv_obj_set_pos(obj, 212, 454);
            lv_obj_set_size(obj, 600, 28);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x657786), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "Check RPM before selecting direction.");
        }
    }
    
    tick_screen_motor_manual();
}

void tick_screen_motor_manual() {
    void *flowState = getFlowState(0, 2);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 4, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_manual_selected_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_manual_selected_value;
            lv_label_set_text(objects.motor_manual_selected_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 8, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_manual_rpm_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_manual_rpm_value;
            lv_label_set_text(objects.motor_manual_rpm_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        int32_t new_val = evalIntegerProperty(flowState, 10, 3, "Failed to evaluate Value in Slider widget");
        int32_t cur_val = lv_slider_get_value(objects.motor_manual_rpm_slider);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.motor_manual_rpm_slider;
            lv_slider_set_value(objects.motor_manual_rpm_slider, new_val, LV_ANIM_OFF);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 12, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_manual_direction_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_manual_direction_value;
            lv_label_set_text(objects.motor_manual_direction_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 13, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.motor_manual_operation_value);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.motor_manual_operation_value;
            lv_label_set_text(objects.motor_manual_operation_value, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 22, 3, "Failed to evaluate Checked state");
        bool cur_val = lv_obj_has_state(objects.motor_manual_run_lamp, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.motor_manual_run_lamp;
            if (new_val) {
                lv_obj_add_state(objects.motor_manual_run_lamp, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.motor_manual_run_lamp, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
    {
        bool new_val = evalBooleanProperty(flowState, 24, 3, "Failed to evaluate Checked state");
        bool cur_val = lv_obj_has_state(objects.motor_manual_stop_lamp, LV_STATE_CHECKED);
        if (new_val != cur_val) {
            tick_value_change_obj = objects.motor_manual_stop_lamp;
            if (new_val) {
                lv_obj_add_state(objects.motor_manual_stop_lamp, LV_STATE_CHECKED);
            } else {
                lv_obj_remove_state(objects.motor_manual_stop_lamp, LV_STATE_CHECKED);
            }
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_system_setting() {
    // system_setting_screen
    void *flowState = getFlowState(0, 3);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.system_setting = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x091018), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // main_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_status = obj;
            lv_obj_set_pos(obj, 348, 19);
            lv_obj_set_size(obj, 330, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM SETTING  /  시비량 설정");
        }
        {
            // main_stop
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_stop = obj;
            lv_obj_set_pos(obj, 844, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xc62828), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 20);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // main_tab_ridge
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_tab_ridge = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 180, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_tab_ridge, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_tab_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_tab_ridge_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 168, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 설정");
                }
            }
        }
        {
            // main_tab_line_ridge
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_tab_line_ridge = obj;
            lv_obj_set_pos(obj, 24, 110);
            lv_obj_set_size(obj, 180, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_tab_prescription
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_tab_prescription = obj;
            lv_obj_set_pos(obj, 210, 64);
            lv_obj_set_size(obj, 194, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_tab_prescription, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1b2731), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_tab_prescription_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_tab_prescription_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 182, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "시비량 설정");
                }
            }
        }
        {
            // main_tab_line_prescription
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_tab_line_prescription = obj;
            lv_obj_set_pos(obj, 210, 110);
            lv_obj_set_size(obj, 194, 4);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_tab_preparation
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_tab_preparation = obj;
            lv_obj_set_pos(obj, 410, 64);
            lv_obj_set_size(obj, 186, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_tab_preparation, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_tab_preparation_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_tab_preparation_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 174, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "준비량");
                }
            }
        }
        {
            // main_tab_line_preparation
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_tab_line_preparation = obj;
            lv_obj_set_pos(obj, 410, 110);
            lv_obj_set_size(obj, 186, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_tab_work
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_tab_work = obj;
            lv_obj_set_pos(obj, 602, 64);
            lv_obj_set_size(obj, 176, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_tab_work, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_tab_work_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_tab_work_label = obj;
                    lv_obj_set_pos(obj, 0, 3);
                    lv_obj_set_size(obj, 164, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "작업");
                }
            }
        }
        {
            // main_tab_line_work
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_tab_line_work = obj;
            lv_obj_set_pos(obj, 602, 89);
            lv_obj_set_size(obj, 176, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_tab_weighing
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_tab_weighing = obj;
            lv_obj_set_pos(obj, 784, 64);
            lv_obj_set_size(obj, 216, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_tab_weighing, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_tab_weighing_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_tab_weighing_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 204, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "계량시험");
                }
            }
        }
        {
            // main_tab_line_weighing
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_tab_line_weighing = obj;
            lv_obj_set_pos(obj, 784, 110);
            lv_obj_set_size(obj, 216, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_content
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.main_content = obj;
            lv_obj_set_pos(obj, 25, 132);
            lv_obj_set_size(obj, 976, 442);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_right_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_right_title = obj;
            lv_obj_set_pos(obj, 30, 150);
            lv_obj_set_size(obj, 440, 38);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "시비량 설정");
        }
        {
            // main_nitrogen
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_nitrogen = obj;
            lv_obj_set_pos(obj, 30, 198);
            lv_obj_set_size(obj, 214, 44);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_add_state(obj, LV_STATE_CHECKED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x25313b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe28a), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xc58e00), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff3c4), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_opa(obj, 48, LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_nitrogen_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_nitrogen_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 202, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "질소 처방");
                }
            }
        }
        {
            // main_direct
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_direct = obj;
            lv_obj_set_pos(obj, 256, 198);
            lv_obj_set_size(obj, 222, 44);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe28a), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xc58e00), LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff3c4), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
            lv_obj_set_style_shadow_opa(obj, 48, LV_PART_MAIN | LV_STATE_CHECKED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_direct_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_direct_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 210, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "제품량 직접 입력");
                }
            }
        }
        {
            // main_nitrogen_amount_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_nitrogen_amount_label = obj;
            lv_obj_set_pos(obj, 30, 256);
            lv_obj_set_size(obj, 170, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "이번 작업 질소량");
        }
        {
            // recipe_nitrogen_rate_input
            lv_obj_t *obj = lv_textarea_create(parent_obj);
            objects.recipe_nitrogen_rate_input = obj;
            lv_obj_set_pos(obj, 204, 250);
            lv_obj_set_size(obj, 274, 39);
            lv_textarea_set_accepted_chars(obj, "0123456789.");
            lv_textarea_set_max_length(obj, 8);
            lv_textarea_set_placeholder_text(obj, "0");
            lv_textarea_set_one_line(obj, true);
            lv_textarea_set_password_mode(obj, false);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_recipe_nitrogen_rate_input, LV_EVENT_ALL, flowState);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // main_nitrogen_unit
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_nitrogen_unit = obj;
            lv_obj_set_pos(obj, 368, 256);
            lv_obj_set_size(obj, 102, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "kg N/10a");
        }
        {
            // main_upper_ratio_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_upper_ratio_label = obj;
            lv_obj_set_pos(obj, 30, 312);
            lv_obj_set_size(obj, 110, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "전층 배분");
        }
        {
            // main_upper_ratio
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_upper_ratio = obj;
            lv_obj_set_pos(obj, 142, 306);
            lv_obj_set_size(obj, 126, 44);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "입력   %");
        }
        {
            // main_lower_ratio_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_lower_ratio_label = obj;
            lv_obj_set_pos(obj, 282, 312);
            lv_obj_set_size(obj, 58, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "지중");
        }
        {
            // main_lower_ratio
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_lower_ratio = obj;
            lv_obj_set_pos(obj, 340, 306);
            lv_obj_set_size(obj, 138, 44);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x687782), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "100-전층   %");
        }
        {
            // main_upper_product_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_upper_product_label = obj;
            lv_obj_set_pos(obj, 30, 372);
            lv_obj_set_size(obj, 108, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "전층 A+B");
        }
        {
            // main_upper_product
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_upper_product = obj;
            lv_obj_set_pos(obj, 142, 362);
            lv_obj_set_size(obj, 336, 48);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "제품 선택 / 찾기  ▼");
        }
        {
            // main_lower_product_label
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_lower_product_label = obj;
            lv_obj_set_pos(obj, 30, 432);
            lv_obj_set_size(obj, 108, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "지중 C+D");
        }
        {
            // main_lower_product
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.main_lower_product = obj;
            lv_obj_set_pos(obj, 142, 422);
            lv_obj_set_size(obj, 336, 48);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "제품 선택 / 찾기  ▼");
        }
        {
            // fertilizer_temp_save
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.fertilizer_temp_save = obj;
            lv_obj_set_pos(obj, 30, 486);
            lv_obj_set_size(obj, 210, 46);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // fertilizer_temp_save_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.fertilizer_temp_save_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 198, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "임시 저장");
                }
            }
        }
        {
            // fertilizer_save_settings
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.fertilizer_save_settings = obj;
            lv_obj_set_pos(obj, 254, 486);
            lv_obj_set_size(obj, 224, 46);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd89a16), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // fertilizer_save_settings_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.fertilizer_save_settings_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 212, 27);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x191506), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "설정 저장");
                }
            }
        }
        {
            // main_stop_1
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.main_stop_1 = obj;
            lv_obj_set_pos(obj, 24, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_system_setting_main_stop_1, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd89a16), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // main_stop_label_1
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.main_stop_label_1 = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 20);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x191506), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "DASHBOARD");
                }
            }
        }
        {
            // ridge_divider_1
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_divider_1 = obj;
            lv_obj_set_pos(obj, 513, 152);
            lv_obj_set_size(obj, 1, 402);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    
    tick_screen_system_setting();
}

void tick_screen_system_setting() {
    void *flowState = getFlowState(0, 3);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 26, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.recipe_nitrogen_rate_input);
        uint32_t max_length = lv_textarea_get_max_length(objects.recipe_nitrogen_rate_input);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.recipe_nitrogen_rate_input;
            lv_textarea_set_text(objects.recipe_nitrogen_rate_input, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_ridge_setup() {
    // ridge_screen
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.ridge_setup = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x091018), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // ridge_app_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ridge_app_title = obj;
            lv_obj_set_pos(obj, 24, 14);
            lv_obj_set_size(obj, 250, 42);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101820), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "DAJILOG");
        }
        {
            // ridge_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.ridge_status = obj;
            lv_obj_set_pos(obj, 348, 19);
            lv_obj_set_size(obj, 330, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "SYSTEM SETTING  /  두둑 설정");
        }
        {
            // ridge_stop
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_stop = obj;
            lv_obj_set_pos(obj, 844, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xc62828), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 28);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // ridge_tab_ridge
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_tab_ridge = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 180, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_tab_ridge, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x1b2731), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_tab_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_tab_ridge_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 168, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 설정");
                }
            }
        }
        {
            // ridge_tab_line_ridge
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_tab_line_ridge = obj;
            lv_obj_set_pos(obj, 24, 110);
            lv_obj_set_size(obj, 180, 4);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ridge_tab_prescription
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_tab_prescription = obj;
            lv_obj_set_pos(obj, 210, 64);
            lv_obj_set_size(obj, 194, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_tab_prescription, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_tab_prescription_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_tab_prescription_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 182, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "시비량 설정");
                }
            }
        }
        {
            // ridge_tab_line_prescription
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_tab_line_prescription = obj;
            lv_obj_set_pos(obj, 210, 110);
            lv_obj_set_size(obj, 194, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ridge_tab_preparation
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_tab_preparation = obj;
            lv_obj_set_pos(obj, 410, 64);
            lv_obj_set_size(obj, 186, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_tab_preparation, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_tab_preparation_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_tab_preparation_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 174, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "준비량");
                }
            }
        }
        {
            // ridge_tab_line_preparation
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_tab_line_preparation = obj;
            lv_obj_set_pos(obj, 410, 110);
            lv_obj_set_size(obj, 186, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ridge_tab_work
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_tab_work = obj;
            lv_obj_set_pos(obj, 602, 64);
            lv_obj_set_size(obj, 176, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_tab_work, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_tab_work_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_tab_work_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 164, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "작업");
                }
            }
        }
        {
            // ridge_tab_line_work
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_tab_line_work = obj;
            lv_obj_set_pos(obj, 602, 110);
            lv_obj_set_size(obj, 176, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ridge_tab_weighing
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.ridge_tab_weighing = obj;
            lv_obj_set_pos(obj, 784, 64);
            lv_obj_set_size(obj, 216, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_tab_weighing, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_tab_weighing_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_tab_weighing_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 204, 26);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "계량시험");
                }
            }
        }
        {
            // ridge_tab_line_weighing
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_tab_line_weighing = obj;
            lv_obj_set_pos(obj, 784, 110);
            lv_obj_set_size(obj, 216, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // ridge_content
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.ridge_content = obj;
            lv_obj_set_pos(obj, 24, 132);
            lv_obj_set_size(obj, 976, 442);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111922), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // ridge_left_title
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_left_title = obj;
                    lv_obj_set_pos(obj, 19, 12);
                    lv_obj_set_size(obj, 435, 38);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑과 구간");
                }
                {
                    // ridge_divider
                    lv_obj_t *obj = lv_obj_create(parent_obj);
                    objects.ridge_divider = obj;
                    lv_obj_set_pos(obj, 477, 15);
                    lv_obj_set_size(obj, 1, 402);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x293540), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // ridge_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_ridge_label = obj;
                    lv_obj_set_pos(obj, 21, 64);
                    lv_obj_set_size(obj, 76, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑");
                }
                {
                    // ridge_ridge_value
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_ridge_value = obj;
                    lv_obj_set_pos(obj, 99, 60);
                    lv_obj_set_size(obj, 130, 42);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "B01  ▼");
                }
                {
                    // ridge_count_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_count_label = obj;
                    lv_obj_set_pos(obj, 251, 64);
                    lv_obj_set_size(obj, 92, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 수");
                }
                {
                    // ridge_recipe_ridge_count_input
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.ridge_recipe_ridge_count_input = obj;
                    lv_obj_set_pos(obj, 347, 60);
                    lv_obj_set_size(obj, 82, 39);
                    lv_textarea_set_accepted_chars(obj, "0123456789.");
                    lv_textarea_set_max_length(obj, 3);
                    lv_textarea_set_placeholder_text(obj, "0");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_recipe_ridge_count_input, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // ridge_count_unit
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_count_unit = obj;
                    lv_obj_set_pos(obj, 431, 64);
                    lv_obj_set_size(obj, 36, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "개");
                }
                {
                    // ridge_width_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_width_label = obj;
                    lv_obj_set_pos(obj, 21, 116);
                    lv_obj_set_size(obj, 92, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 폭");
                }
                {
                    // ridge_recipe_ridge_width_input
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.ridge_recipe_ridge_width_input = obj;
                    lv_obj_set_pos(obj, 117, 112);
                    lv_obj_set_size(obj, 112, 39);
                    lv_textarea_set_accepted_chars(obj, "0123456789.");
                    lv_textarea_set_max_length(obj, 8);
                    lv_textarea_set_placeholder_text(obj, "0");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_recipe_ridge_width_input, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // ridge_width_unit
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_width_unit = obj;
                    lv_obj_set_pos(obj, 231, 116);
                    lv_obj_set_size(obj, 32, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "m");
                }
                {
                    // ridge_length_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_length_label = obj;
                    lv_obj_set_pos(obj, 267, 116);
                    lv_obj_set_size(obj, 98, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "실제 길이");
                }
                {
                    // ridge_recipe_actual_length_input
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.ridge_recipe_actual_length_input = obj;
                    lv_obj_set_pos(obj, 367, 112);
                    lv_obj_set_size(obj, 72, 39);
                    lv_textarea_set_accepted_chars(obj, "0123456789.");
                    lv_textarea_set_max_length(obj, 8);
                    lv_textarea_set_placeholder_text(obj, "0");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, event_handler_cb_ridge_setup_ridge_recipe_actual_length_input, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 7, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // ridge_length_unit
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_length_unit = obj;
                    lv_obj_set_pos(obj, 441, 116);
                    lv_obj_set_size(obj, 28, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "m");
                }
                {
                    // ridge_area_only
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_area_only = obj;
                    lv_obj_set_pos(obj, 21, 168);
                    lv_obj_set_size(obj, 208, 44);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_add_state(obj, LV_STATE_CHECKED);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe28a), LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xc58e00), LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff3c4), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_opa(obj, 48, LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_area_only_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_area_only_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 196, 36);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "두둑 면적만");
                        }
                    }
                }
                {
                    // ridge_area_furrow
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_area_furrow = obj;
                    lv_obj_set_pos(obj, 243, 168);
                    lv_obj_set_size(obj, 210, 44);
                    lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffe28a), LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xc58e00), LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_PRESSED);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff3c4), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_border_width(obj, 3, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_color(obj, lv_color_hex(0xe0a800), LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_width(obj, 5, LV_PART_MAIN | LV_STATE_CHECKED);
                    lv_obj_set_style_shadow_opa(obj, 48, LV_PART_MAIN | LV_STATE_CHECKED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_area_furrow_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_area_furrow_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 198, 27);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "두둑+고랑");
                        }
                    }
                }
                {
                    // ridge_all
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_all = obj;
                    lv_obj_set_pos(obj, 21, 224);
                    lv_obj_set_size(obj, 128, 44);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x25313b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_all_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_all_label = obj;
                            lv_obj_set_pos(obj, 0, -5);
                            lv_obj_set_size(obj, 116, 26);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "✓ 전체");
                        }
                    }
                }
                {
                    // ridge_split
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_split = obj;
                    lv_obj_set_pos(obj, 161, 224);
                    lv_obj_set_size(obj, 142, 44);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_split_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_split_label = obj;
                            lv_obj_set_pos(obj, 0, -5);
                            lv_obj_set_size(obj, 130, 26);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "분할");
                        }
                    }
                }
                {
                    // ridge_apply_length
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_apply_length = obj;
                    lv_obj_set_pos(obj, 315, 224);
                    lv_obj_set_size(obj, 138, 44);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_apply_length_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_apply_length_label = obj;
                            lv_obj_set_pos(obj, 0, -5);
                            lv_obj_set_size(obj, 126, 26);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "길이 적용");
                        }
                    }
                }
                {
                    // ridge_north_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_north_label = obj;
                    lv_obj_set_pos(obj, 21, 286);
                    lv_obj_set_size(obj, 64, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "북측");
                }
                {
                    // ridge_north_value
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_north_value = obj;
                    lv_obj_set_pos(obj, 85, 282);
                    lv_obj_set_size(obj, 144, 42);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x687782), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "—    m");
                }
                {
                    // ridge_south_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_south_label = obj;
                    lv_obj_set_pos(obj, 251, 286);
                    lv_obj_set_size(obj, 64, 34);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "남측");
                }
                {
                    // ridge_south_value
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.ridge_south_value = obj;
                    lv_obj_set_pos(obj, 315, 282);
                    lv_obj_set_size(obj, 138, 42);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x687782), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "—    m");
                }
                {
                    // ridge_temp_save
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_temp_save = obj;
                    lv_obj_set_pos(obj, 21, 348);
                    lv_obj_set_size(obj, 190, 46);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x16212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x364655), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x21303c), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_temp_save_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_temp_save_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 178, 27);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xeef5fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "임시 저장");
                        }
                    }
                }
                {
                    // ridge_save_settings
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.ridge_save_settings = obj;
                    lv_obj_set_pos(obj, 227, 348);
                    lv_obj_set_size(obj, 226, 46);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0xf9b938), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xd89a16), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // ridge_save_settings_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.ridge_save_settings_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 214, 25);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x191506), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "설정 저장");
                        }
                    }
                }
            }
        }
    }
    
    tick_screen_ridge_setup();
}

void tick_screen_ridge_setup() {
    void *flowState = getFlowState(0, 4);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 26, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.ridge_recipe_ridge_count_input);
        uint32_t max_length = lv_textarea_get_max_length(objects.ridge_recipe_ridge_count_input);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.ridge_recipe_ridge_count_input;
            lv_textarea_set_text(objects.ridge_recipe_ridge_count_input, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 29, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.ridge_recipe_ridge_width_input);
        uint32_t max_length = lv_textarea_get_max_length(objects.ridge_recipe_ridge_width_input);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.ridge_recipe_ridge_width_input;
            lv_textarea_set_text(objects.ridge_recipe_ridge_width_input, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 32, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.ridge_recipe_actual_length_input);
        uint32_t max_length = lv_textarea_get_max_length(objects.ridge_recipe_actual_length_input);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.ridge_recipe_actual_length_input;
            lv_textarea_set_text(objects.ridge_recipe_actual_length_input, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

void create_screen_preparation() {
    // prep_screen
    void *flowState = getFlowState(0, 5);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.preparation = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf5f7f8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // prep_app_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.prep_app_title = obj;
            lv_obj_set_pos(obj, 24, 14);
            lv_obj_set_size(obj, 250, 42);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101820), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "시비 제어기");
        }
        {
            // prep_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.prep_status = obj;
            lv_obj_set_pos(obj, 348, 19);
            lv_obj_set_size(obj, 330, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x26323b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "설정 중 / 입력 예시");
        }
        {
            // prep_stop
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_stop = obj;
            lv_obj_set_pos(obj, 844, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xc62828), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // prep_tab_ridge
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_tab_ridge = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 180, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_preparation_prep_tab_ridge, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_tab_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_tab_ridge_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 168, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 설정");
                }
            }
        }
        {
            // prep_tab_line_ridge
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_tab_line_ridge = obj;
            lv_obj_set_pos(obj, 24, 110);
            lv_obj_set_size(obj, 180, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_tab_prescription
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_tab_prescription = obj;
            lv_obj_set_pos(obj, 210, 64);
            lv_obj_set_size(obj, 194, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_preparation_prep_tab_prescription, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_tab_prescription_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_tab_prescription_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 182, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "처방과 비료");
                }
            }
        }
        {
            // prep_tab_line_prescription
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_tab_line_prescription = obj;
            lv_obj_set_pos(obj, 210, 110);
            lv_obj_set_size(obj, 194, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_tab_preparation
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_tab_preparation = obj;
            lv_obj_set_pos(obj, 410, 64);
            lv_obj_set_size(obj, 186, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_preparation_prep_tab_preparation, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff7dc), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_tab_preparation_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_tab_preparation_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 174, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "준비량");
                }
            }
        }
        {
            // prep_tab_line_preparation
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_tab_line_preparation = obj;
            lv_obj_set_pos(obj, 410, 110);
            lv_obj_set_size(obj, 186, 4);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_tab_work
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_tab_work = obj;
            lv_obj_set_pos(obj, 602, 64);
            lv_obj_set_size(obj, 176, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_preparation_prep_tab_work, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_tab_work_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_tab_work_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 164, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "작업");
                }
            }
        }
        {
            // prep_tab_line_work
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_tab_line_work = obj;
            lv_obj_set_pos(obj, 602, 110);
            lv_obj_set_size(obj, 176, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_tab_weighing
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.prep_tab_weighing = obj;
            lv_obj_set_pos(obj, 784, 64);
            lv_obj_set_size(obj, 216, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_preparation_prep_tab_weighing, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // prep_tab_weighing_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.prep_tab_weighing_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 204, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "계량시험");
                }
            }
        }
        {
            // prep_tab_line_weighing
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_tab_line_weighing = obj;
            lv_obj_set_pos(obj, 784, 110);
            lv_obj_set_size(obj, 216, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_content
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.prep_content = obj;
            lv_obj_set_pos(obj, 24, 132);
            lv_obj_set_size(obj, 976, 430);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xd7dce0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // prep_content_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.prep_content_title = obj;
            lv_obj_set_pos(obj, 52, 158);
            lv_obj_set_size(obj, 920, 45);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "비료 준비량");
        }
        {
            // prep_placeholder
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.prep_placeholder = obj;
            lv_obj_set_pos(obj, 52, 270);
            lv_obj_set_size(obj, 920, 52);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x7b8790), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "화면 준비 중");
        }
        {
            // prep_guide
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.prep_guide = obj;
            lv_obj_set_pos(obj, 52, 335);
            lv_obj_set_size(obj, 920, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8d979e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "이 탭의 세부 기능은 다음 단계에서 추가합니다.");
        }
    }
    
    tick_screen_preparation();
}

void tick_screen_preparation() {
    void *flowState = getFlowState(0, 5);
    (void)flowState;
}

void create_screen_work() {
    // workpage_screen
    void *flowState = getFlowState(0, 6);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.work = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf5f7f8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // workpage_app_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.workpage_app_title = obj;
            lv_obj_set_pos(obj, 24, 14);
            lv_obj_set_size(obj, 250, 42);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101820), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "시비 제어기");
        }
        {
            // workpage_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.workpage_status = obj;
            lv_obj_set_pos(obj, 348, 19);
            lv_obj_set_size(obj, 330, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x26323b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "설정 중 / 입력 예시");
        }
        {
            // workpage_stop
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_stop = obj;
            lv_obj_set_pos(obj, 844, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xc62828), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // workpage_tab_ridge
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_tab_ridge = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 180, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_work_workpage_tab_ridge, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_tab_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_tab_ridge_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 168, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 설정");
                }
            }
        }
        {
            // workpage_tab_line_ridge
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_tab_line_ridge = obj;
            lv_obj_set_pos(obj, 24, 110);
            lv_obj_set_size(obj, 180, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_tab_prescription
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_tab_prescription = obj;
            lv_obj_set_pos(obj, 210, 64);
            lv_obj_set_size(obj, 194, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_work_workpage_tab_prescription, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_tab_prescription_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_tab_prescription_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 182, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "처방과 비료");
                }
            }
        }
        {
            // workpage_tab_line_prescription
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_tab_line_prescription = obj;
            lv_obj_set_pos(obj, 210, 110);
            lv_obj_set_size(obj, 194, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_tab_preparation
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_tab_preparation = obj;
            lv_obj_set_pos(obj, 410, 64);
            lv_obj_set_size(obj, 186, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_work_workpage_tab_preparation, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_tab_preparation_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_tab_preparation_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 174, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "준비량");
                }
            }
        }
        {
            // workpage_tab_line_preparation
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_tab_line_preparation = obj;
            lv_obj_set_pos(obj, 410, 110);
            lv_obj_set_size(obj, 186, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_tab_work
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_tab_work = obj;
            lv_obj_set_pos(obj, 602, 64);
            lv_obj_set_size(obj, 176, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_work_workpage_tab_work, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff7dc), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_tab_work_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_tab_work_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 164, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "작업");
                }
            }
        }
        {
            // workpage_tab_line_work
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_tab_line_work = obj;
            lv_obj_set_pos(obj, 602, 110);
            lv_obj_set_size(obj, 176, 4);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_tab_weighing
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.workpage_tab_weighing = obj;
            lv_obj_set_pos(obj, 784, 64);
            lv_obj_set_size(obj, 216, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_work_workpage_tab_weighing, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // workpage_tab_weighing_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.workpage_tab_weighing_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 204, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "계량시험");
                }
            }
        }
        {
            // workpage_tab_line_weighing
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_tab_line_weighing = obj;
            lv_obj_set_pos(obj, 784, 110);
            lv_obj_set_size(obj, 216, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_content
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.workpage_content = obj;
            lv_obj_set_pos(obj, 24, 132);
            lv_obj_set_size(obj, 976, 430);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xd7dce0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // workpage_content_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.workpage_content_title = obj;
            lv_obj_set_pos(obj, 52, 158);
            lv_obj_set_size(obj, 920, 45);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "작업");
        }
        {
            // workpage_placeholder
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.workpage_placeholder = obj;
            lv_obj_set_pos(obj, 52, 270);
            lv_obj_set_size(obj, 920, 52);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x7b8790), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "화면 준비 중");
        }
        {
            // workpage_guide
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.workpage_guide = obj;
            lv_obj_set_pos(obj, 52, 335);
            lv_obj_set_size(obj, 920, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8d979e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "이 탭의 세부 기능은 다음 단계에서 추가합니다.");
        }
    }
    
    tick_screen_work();
}

void tick_screen_work() {
    void *flowState = getFlowState(0, 6);
    (void)flowState;
}

void create_screen_weighing_test() {
    // weigh_screen
    void *flowState = getFlowState(0, 7);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.weighing_test = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf5f7f8), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // weigh_app_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weigh_app_title = obj;
            lv_obj_set_pos(obj, 24, 14);
            lv_obj_set_size(obj, 250, 42);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x101820), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "시비 제어기");
        }
        {
            // weigh_status
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weigh_status = obj;
            lv_obj_set_pos(obj, 348, 19);
            lv_obj_set_size(obj, 330, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x26323b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "설정 중 / 입력 예시");
        }
        {
            // weigh_stop
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_stop = obj;
            lv_obj_set_pos(obj, 844, 12);
            lv_obj_set_size(obj, 156, 44);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xc62828), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x8e0000), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_stop_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_stop_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 144, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "전체 정지");
                }
            }
        }
        {
            // weigh_tab_ridge
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_tab_ridge = obj;
            lv_obj_set_pos(obj, 24, 64);
            lv_obj_set_size(obj, 180, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_weighing_test_weigh_tab_ridge, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_tab_ridge_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_tab_ridge_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 168, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "두둑 설정");
                }
            }
        }
        {
            // weigh_tab_line_ridge
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_tab_line_ridge = obj;
            lv_obj_set_pos(obj, 24, 110);
            lv_obj_set_size(obj, 180, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_tab_prescription
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_tab_prescription = obj;
            lv_obj_set_pos(obj, 210, 64);
            lv_obj_set_size(obj, 194, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_weighing_test_weigh_tab_prescription, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_tab_prescription_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_tab_prescription_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 182, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "처방과 비료");
                }
            }
        }
        {
            // weigh_tab_line_prescription
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_tab_line_prescription = obj;
            lv_obj_set_pos(obj, 210, 110);
            lv_obj_set_size(obj, 194, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_tab_preparation
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_tab_preparation = obj;
            lv_obj_set_pos(obj, 410, 64);
            lv_obj_set_size(obj, 186, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_weighing_test_weigh_tab_preparation, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_tab_preparation_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_tab_preparation_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 174, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "준비량");
                }
            }
        }
        {
            // weigh_tab_line_preparation
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_tab_line_preparation = obj;
            lv_obj_set_pos(obj, 410, 110);
            lv_obj_set_size(obj, 186, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_tab_work
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_tab_work = obj;
            lv_obj_set_pos(obj, 602, 64);
            lv_obj_set_size(obj, 176, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_weighing_test_weigh_tab_work, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x9aa2a8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_tab_work_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_tab_work_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 164, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "작업");
                }
            }
        }
        {
            // weigh_tab_line_work
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_tab_line_work = obj;
            lv_obj_set_pos(obj, 602, 110);
            lv_obj_set_size(obj, 176, 1);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xd1d5d8), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_tab_weighing
            lv_obj_t *obj = lv_button_create(parent_obj);
            objects.weigh_tab_weighing = obj;
            lv_obj_set_pos(obj, 784, 64);
            lv_obj_set_size(obj, 216, 44);
            lv_obj_add_event_cb(obj, event_handler_cb_weighing_test_weigh_tab_weighing, LV_EVENT_ALL, flowState);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xfff7dc), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_PRESSED);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // weigh_tab_weighing_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.weigh_tab_weighing_label = obj;
                    lv_obj_set_pos(obj, 0, 0);
                    lv_obj_set_size(obj, 204, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "계량시험");
                }
            }
        }
        {
            // weigh_tab_line_weighing
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_tab_line_weighing = obj;
            lv_obj_set_pos(obj, 784, 110);
            lv_obj_set_size(obj, 216, 4);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_content
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.weigh_content = obj;
            lv_obj_set_pos(obj, 24, 132);
            lv_obj_set_size(obj, 976, 430);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0xd7dce0), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        {
            // weigh_content_title
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weigh_content_title = obj;
            lv_obj_set_pos(obj, 52, 158);
            lv_obj_set_size(obj, 920, 45);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "계량시험");
        }
        {
            // weigh_placeholder
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weigh_placeholder = obj;
            lv_obj_set_pos(obj, 52, 270);
            lv_obj_set_size(obj, 920, 52);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x7b8790), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "화면 준비 중");
        }
        {
            // weigh_guide
            lv_obj_t *obj = lv_label_create(parent_obj);
            objects.weigh_guide = obj;
            lv_obj_set_pos(obj, 52, 335);
            lv_obj_set_size(obj, 920, 34);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(obj, lv_color_hex(0x8d979e), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_static(obj, "이 탭의 세부 기능은 다음 단계에서 추가합니다.");
        }
    }
    
    tick_screen_weighing_test();
}

void tick_screen_weighing_test() {
    void *flowState = getFlowState(0, 7);
    (void)flowState;
}

void create_screen_numeric_input() {
    // numeric_input_screen
    void *flowState = getFlowState(0, 8);
    (void)flowState;
    lv_obj_t *obj = lv_obj_create(0);
    objects.numeric_input = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 1024, 600);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xedf1f4), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        lv_obj_t *parent_obj = obj;
        {
            // numeric_input_card
            lv_obj_t *obj = lv_obj_create(parent_obj);
            objects.numeric_input_card = obj;
            lv_obj_set_pos(obj, 170, 59);
            lv_obj_set_size(obj, 684, 482);
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
            lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(obj, lv_color_hex(0x46515b), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                lv_obj_t *parent_obj = obj;
                {
                    // numeric_input_title_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.numeric_input_title_label = obj;
                    lv_obj_set_pos(obj, 32, 20);
                    lv_obj_set_size(obj, 620, 36);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // numeric_input_editor
                    lv_obj_t *obj = lv_textarea_create(parent_obj);
                    objects.numeric_input_editor = obj;
                    lv_obj_set_pos(obj, 50, 70);
                    lv_obj_set_size(obj, 584, 53);
                    lv_textarea_set_accepted_chars(obj, "0123456789.");
                    lv_textarea_set_max_length(obj, 12);
                    lv_textarea_set_placeholder_text(obj, "0");
                    lv_textarea_set_one_line(obj, true);
                    lv_textarea_set_password_mode(obj, false);
                    lv_obj_add_event_cb(obj, event_handler_cb_numeric_input_numeric_input_editor, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr28, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xf7f9fa), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_color(obj, lv_color_hex(0x1565c0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    // numeric_input_unit_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.numeric_input_unit_label = obj;
                    lv_obj_set_pos(obj, 50, 139);
                    lv_obj_set_size(obj, 584, 28);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x65717d), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text(obj, "");
                }
                {
                    // numeric_input_keypad
                    lv_obj_t *obj = lv_keyboard_create(parent_obj);
                    objects.numeric_input_keypad = obj;
                    lv_obj_set_pos(obj, 2, -111);
                    lv_obj_set_size(obj, 584, 191);
                    lv_keyboard_set_mode(obj, LV_KEYBOARD_MODE_NUMBER);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xe8edf1), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_top(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_bottom(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_left(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_pad_right(obj, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x17212b), LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 7, LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_font(obj, &lv_font_montserrat_28, LV_PART_ITEMS | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xbbdefb), LV_PART_ITEMS | LV_STATE_PRESSED);
                }
                {
                    // numeric_input_cancel_button
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.numeric_input_cancel_button = obj;
                    lv_obj_set_pos(obj, 95, 377);
                    lv_obj_set_size(obj, 210, 58);
                    lv_obj_add_event_cb(obj, event_handler_cb_numeric_input_numeric_input_cancel_button, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xe1e5e8), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0xc7cdd2), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // numeric_input_cancel_button_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.numeric_input_cancel_button_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 198, 28);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0x26323b), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "취소");
                        }
                    }
                }
                {
                    // numeric_input_done_button
                    lv_obj_t *obj = lv_button_create(parent_obj);
                    objects.numeric_input_done_button = obj;
                    lv_obj_set_pos(obj, 376, 377);
                    lv_obj_set_size(obj, 210, 58);
                    lv_obj_add_event_cb(obj, event_handler_cb_numeric_input_numeric_input_done_button, LV_EVENT_ALL, flowState);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1565c0), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_bg_color(obj, lv_color_hex(0x0d47a1), LV_PART_MAIN | LV_STATE_PRESSED);
                    {
                        lv_obj_t *parent_obj = obj;
                        {
                            // numeric_input_done_button_label
                            lv_obj_t *obj = lv_label_create(parent_obj);
                            objects.numeric_input_done_button_label = obj;
                            lv_obj_set_pos(obj, 0, 0);
                            lv_obj_set_size(obj, 198, 28);
                            lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                            lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr20, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_color(obj, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_obj_set_style_align(obj, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                            lv_label_set_text_static(obj, "완료");
                        }
                    }
                }
                {
                    // numeric_input_help_label
                    lv_obj_t *obj = lv_label_create(parent_obj);
                    objects.numeric_input_help_label = obj;
                    lv_obj_set_pos(obj, 32, 445);
                    lv_obj_set_size(obj, 620, 20);
                    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW);
                    lv_obj_set_style_text_font(obj, &ui_font_maru_buri_regular_kr16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(obj, lv_color_hex(0x65717d), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_label_set_text_static(obj, "숫자를 입력한 뒤 완료를 누르세요.");
                }
            }
        }
    }
    lv_keyboard_set_textarea(objects.numeric_input_keypad, objects.numeric_input_editor);
    
    tick_screen_numeric_input();
}

void tick_screen_numeric_input() {
    void *flowState = getFlowState(0, 8);
    (void)flowState;
    {
        const char *new_val = evalTextProperty(flowState, 2, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.numeric_input_title_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.numeric_input_title_label;
            lv_label_set_text(objects.numeric_input_title_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 3, 3, "Failed to evaluate Text in Textarea widget");
        const char *cur_val = lv_textarea_get_text(objects.numeric_input_editor);
        uint32_t max_length = lv_textarea_get_max_length(objects.numeric_input_editor);
        if (strncmp(new_val, cur_val, max_length) != 0) {
            tick_value_change_obj = objects.numeric_input_editor;
            lv_textarea_set_text(objects.numeric_input_editor, new_val);
            tick_value_change_obj = NULL;
        }
    }
    {
        const char *new_val = evalTextProperty(flowState, 4, 3, "Failed to evaluate Text in Label widget");
        const char *cur_val = lv_label_get_text(objects.numeric_input_unit_label);
        if (strcmp(new_val, cur_val) != 0) {
            tick_value_change_obj = objects.numeric_input_unit_label;
            lv_label_set_text(objects.numeric_input_unit_label, new_val);
            tick_value_change_obj = NULL;
        }
    }
}

typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_startup_animation,
    tick_screen_dashboard,
    tick_screen_motor_manual,
    tick_screen_system_setting,
    tick_screen_ridge_setup,
    tick_screen_preparation,
    tick_screen_work,
    tick_screen_weighing_test,
    tick_screen_numeric_input,
};
void tick_screen(int screen_index) {
    if (screen_index >= 0 && screen_index < 9) {
        tick_screen_funcs[screen_index]();
    }
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen(screenId - 1);
}

//
// Fonts
//

ext_font_desc_t fonts[] = {
    { "MaruBuri-Regular_KR16", &ui_font_maru_buri_regular_kr16 },
    { "MaruBuri-Regular_KR20", &ui_font_maru_buri_regular_kr20 },
    { "MaruBuri-Regular_KR28", &ui_font_maru_buri_regular_kr28 },
#if LV_FONT_MONTSERRAT_8
    { "MONTSERRAT_8", &lv_font_montserrat_8 },
#endif
#if LV_FONT_MONTSERRAT_10
    { "MONTSERRAT_10", &lv_font_montserrat_10 },
#endif
#if LV_FONT_MONTSERRAT_12
    { "MONTSERRAT_12", &lv_font_montserrat_12 },
#endif
#if LV_FONT_MONTSERRAT_14
    { "MONTSERRAT_14", &lv_font_montserrat_14 },
#endif
#if LV_FONT_MONTSERRAT_16
    { "MONTSERRAT_16", &lv_font_montserrat_16 },
#endif
#if LV_FONT_MONTSERRAT_18
    { "MONTSERRAT_18", &lv_font_montserrat_18 },
#endif
#if LV_FONT_MONTSERRAT_20
    { "MONTSERRAT_20", &lv_font_montserrat_20 },
#endif
#if LV_FONT_MONTSERRAT_22
    { "MONTSERRAT_22", &lv_font_montserrat_22 },
#endif
#if LV_FONT_MONTSERRAT_24
    { "MONTSERRAT_24", &lv_font_montserrat_24 },
#endif
#if LV_FONT_MONTSERRAT_26
    { "MONTSERRAT_26", &lv_font_montserrat_26 },
#endif
#if LV_FONT_MONTSERRAT_28
    { "MONTSERRAT_28", &lv_font_montserrat_28 },
#endif
#if LV_FONT_MONTSERRAT_30
    { "MONTSERRAT_30", &lv_font_montserrat_30 },
#endif
#if LV_FONT_MONTSERRAT_32
    { "MONTSERRAT_32", &lv_font_montserrat_32 },
#endif
#if LV_FONT_MONTSERRAT_34
    { "MONTSERRAT_34", &lv_font_montserrat_34 },
#endif
#if LV_FONT_MONTSERRAT_36
    { "MONTSERRAT_36", &lv_font_montserrat_36 },
#endif
#if LV_FONT_MONTSERRAT_38
    { "MONTSERRAT_38", &lv_font_montserrat_38 },
#endif
#if LV_FONT_MONTSERRAT_40
    { "MONTSERRAT_40", &lv_font_montserrat_40 },
#endif
#if LV_FONT_MONTSERRAT_42
    { "MONTSERRAT_42", &lv_font_montserrat_42 },
#endif
#if LV_FONT_MONTSERRAT_44
    { "MONTSERRAT_44", &lv_font_montserrat_44 },
#endif
#if LV_FONT_MONTSERRAT_46
    { "MONTSERRAT_46", &lv_font_montserrat_46 },
#endif
#if LV_FONT_MONTSERRAT_48
    { "MONTSERRAT_48", &lv_font_montserrat_48 },
#endif
};

//
//
//

void create_screens() {
    
    eez_flow_init_fonts(fonts, sizeof(fonts) / sizeof(ext_font_desc_t));

// Set default LVGL theme
    lv_display_t *dispp = lv_display_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_display_set_theme(dispp, theme);
    
    // Initialize screens
    eez_flow_init_screen_names(screen_names, sizeof(screen_names) / sizeof(const char *));
    eez_flow_init_object_names(object_names, sizeof(object_names) / sizeof(const char *));
    
    // Create screens
    create_screen_startup_animation();
    create_screen_dashboard();
    create_screen_motor_manual();
    create_screen_system_setting();
    create_screen_ridge_setup();
    create_screen_preparation();
    create_screen_work();
    create_screen_weighing_test();
    create_screen_numeric_input();
}