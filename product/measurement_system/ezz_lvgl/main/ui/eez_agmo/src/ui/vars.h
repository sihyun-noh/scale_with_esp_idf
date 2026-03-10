#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations

// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_CHECKBOX_EVT_BOTH = 0,
    FLOW_GLOBAL_VARIABLE_BOTH_TEXT_CHANGE = 1,
    FLOW_GLOBAL_VARIABLE_DRIVER1_EMPTY = 2,
    FLOW_GLOBAL_VARIABLE_DRIVER2_EMPTY = 3,
    FLOW_GLOBAL_VARIABLE_BOTH_SET_RPM_EMPTY = 4,
    FLOW_GLOBAL_VARIABLE_RUN_LV_STATE = 5
};

// Native global variables

extern int32_t get_var_driver1_set_val();
extern void set_var_driver1_set_val(int32_t value);
extern int32_t get_var_driver2_set_val();
extern void set_var_driver2_set_val(int32_t value);
extern int32_t get_var_both_set_rpm_val();
extern void set_var_both_set_rpm_val(int32_t value);
extern int32_t get_var_checked_evt_both_hidden();
extern void set_var_checked_evt_both_hidden(int32_t value);
extern const char *get_var_fb_driver1_r();
extern void set_var_fb_driver1_r(const char *value);
extern const char *get_var_fb_driver1_l();
extern void set_var_fb_driver1_l(const char *value);
extern const char *get_var_fb_driver2_r();
extern void set_var_fb_driver2_r(const char *value);
extern const char *get_var_fb_driver2_l();
extern void set_var_fb_driver2_l(const char *value);
extern int32_t get_var_run_state();
extern void set_var_run_state(int32_t value);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/