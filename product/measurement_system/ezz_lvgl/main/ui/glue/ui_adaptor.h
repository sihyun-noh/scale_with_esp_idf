#ifndef _UI_MSG_H_
#define _UI_MSG_H_

#include <stdint.h>
#include "ui/eez_agmo/src/ui/vars.h"

#ifdef __cplusplus
extern "C" {
#endif

// EEZ native variable hooks
int32_t get_var_driver1_set_val(void);
void set_var_driver1_set_val(int32_t value);
int32_t get_var_driver2_set_val(void);
void set_var_driver2_set_val(int32_t value);
int32_t get_var_both_set_rpm_val(void);
void set_var_both_set_rpm_val(int32_t value);
int32_t get_var_run_state();
void set_var_run_state(int32_t value);

const char *get_var_fb_driver1_r();
void set_var_fb_driver1_r(const char *value);
const char *get_var_fb_driver1_l();
void set_var_fb_driver1_l(const char *value);
const char *get_var_fb_driver2_r();
void set_var_fb_driver2_r(const char *value);
const char *get_var_fb_driver2_l();
void set_var_fb_driver2_l(const char *value);

void set_var__3_axis_max_rpm(const char *value);
const char *get_var__3_axis_max_rpm();
void set_var__3_axis_max_kmh(const char *value);
const char *get_var__3_axis_max_kmh();

#ifdef __cplusplus
}
#endif
#endif
