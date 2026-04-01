
#include <string.h>

#include "ui_adaptor.h"
#include "core/lv_obj_style.h"
#include "ui_bridge.h"
#include "vars.h"

// ---- Local shadow values (UI model) ----
static int32_t driver1_set_val;
static int32_t driver2_set_val;
static int32_t both_set_rpm_val;
int32_t checked_evt_both_hidden;
static int32_t run_state;

static char fb_driver1_r[10] = { 0 };
static char fb_driver1_l[10] = { 0 };
static char fb_driver2_r[10] = { 0 };
static char fb_driver2_l[10] = { 0 };

static char _3_axis_max_kmh[50] = { 0 };
static char _3_axis_max_rpm[50] = { 0 };

// ---------------- UI -> HW ----------------
int32_t get_var_checked_evt_both_hidden() {
  return checked_evt_both_hidden;
}

void set_var_checked_evt_both_hidden(int32_t value) {
  checked_evt_both_hidden = value;
  bridge_send_to_hw(UI_CMD_EVT_BOTH, value, 0);
}

int32_t get_var_run_state() {
  return run_state;
}

void set_var_run_state(int32_t value) {
  run_state = value;
  bridge_send_to_hw(UI_CMD_RUN_STATE, value, 0);
}

int32_t get_var_driver1_set_val(void) {
  return driver1_set_val;
}

void set_var_driver1_set_val(int32_t value) {
  driver1_set_val = value;
  bridge_send_to_hw(UI_CMD_DRIVER1_SET, value, 0);
}

int32_t get_var_driver2_set_val(void) {
  return driver2_set_val;
}

void set_var_driver2_set_val(int32_t value) {
  driver2_set_val = value;
  bridge_send_to_hw(UI_CMD_DRIVER2_SET, value, 0);
}

int32_t get_var_both_set_rpm_val(void) {
  return both_set_rpm_val;
}

void set_var_both_set_rpm_val(int32_t value) {
  both_set_rpm_val = value;
  bridge_send_to_hw(UI_CMD_BOTH_SET_RPM, value, 0);
}

const char *get_var__3_axis_max_kmh() {
  return _3_axis_max_kmh;
}

void set_var__3_axis_max_kmh(const char *value) {
  strncpy(_3_axis_max_kmh, value, sizeof(_3_axis_max_kmh) / sizeof(char));
  _3_axis_max_kmh[sizeof(_3_axis_max_kmh) / sizeof(char) - 1] = 0;
  bridge_send_to_hw_str(UI_CMD_3AXIS_MAX_KMH, _3_axis_max_kmh, 0);
}

const char *get_var__3_axis_max_rpm() {
  return _3_axis_max_rpm;
}

void set_var__3_axis_max_rpm(const char *value) {
  strncpy(_3_axis_max_rpm, value, sizeof(_3_axis_max_rpm) / sizeof(char));
  _3_axis_max_rpm[sizeof(_3_axis_max_rpm) / sizeof(char) - 1] = 0;
  bridge_send_to_hw_str(UI_CMD_3AXIS_MAX_RPM, _3_axis_max_rpm, 0);
}

// ---------------- HW -> UI ----------------

const char *get_var_fb_driver1_r() {
  return fb_driver1_r;
}

void set_var_fb_driver1_r(const char *value) {
  strncpy(fb_driver1_r, value, sizeof(fb_driver1_r) / sizeof(char));
  fb_driver1_r[sizeof(fb_driver1_r) / sizeof(char) - 1] = 0;
}

const char *get_var_fb_driver1_l() {
  return fb_driver1_l;
}

void set_var_fb_driver1_l(const char *value) {
  strncpy(fb_driver1_l, value, sizeof(fb_driver1_l) / sizeof(char));
  fb_driver1_l[sizeof(fb_driver1_l) / sizeof(char) - 1] = 0;
}

const char *get_var_fb_driver2_r() {
  return fb_driver2_r;
}

void set_var_fb_driver2_r(const char *value) {
  strncpy(fb_driver2_r, value, sizeof(fb_driver2_r) / sizeof(char));
  fb_driver2_r[sizeof(fb_driver2_r) / sizeof(char) - 1] = 0;
}

const char *get_var_fb_driver2_l() {
  return fb_driver2_l;
}

void set_var_fb_driver2_l(const char *value) {
  strncpy(fb_driver2_l, value, sizeof(fb_driver2_l) / sizeof(char));
  fb_driver2_l[sizeof(fb_driver2_l) / sizeof(char) - 1] = 0;
}
