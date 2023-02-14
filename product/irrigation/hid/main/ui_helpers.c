// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <stdio.h>

#include "ui.h"
#include "ui_helpers.h"
#include "gui.h"

#include "sys_config.h"

void _ui_bar_set_property(lv_obj_t *target, int id, int val) {
  if (id == _UI_BAR_PROPERTY_VALUE_WITH_ANIM)
    lv_bar_set_value(target, val, LV_ANIM_ON);
  if (id == _UI_BAR_PROPERTY_VALUE)
    lv_bar_set_value(target, val, LV_ANIM_OFF);
}

void _ui_basic_set_property(lv_obj_t *target, int id, int val) {
  if (id == _UI_BASIC_PROPERTY_POSITION_X)
    lv_obj_set_x(target, val);
  if (id == _UI_BASIC_PROPERTY_POSITION_Y)
    lv_obj_set_y(target, val);
  if (id == _UI_BASIC_PROPERTY_WIDTH)
    lv_obj_set_width(target, val);
  if (id == _UI_BASIC_PROPERTY_HEIGHT)
    lv_obj_set_height(target, val);
}

void _ui_dropdown_set_property(lv_obj_t *target, int id, int val) {
  if (id == _UI_DROPDOWN_PROPERTY_SELECTED)
    lv_dropdown_set_selected(target, val);
}

void _ui_image_set_property(lv_obj_t *target, int id, uint8_t *val) {
  if (id == _UI_IMAGE_PROPERTY_IMAGE)
    lv_img_set_src(target, val);
}

void _ui_label_set_property(lv_obj_t *target, int id, char *val) {
  if (id == _UI_LABEL_PROPERTY_TEXT)
    lv_label_set_text(target, val);
}

void _ui_roller_set_property(lv_obj_t *target, int id, int val) {
  if (id == _UI_ROLLER_PROPERTY_SELECTED_WITH_ANIM)
    lv_roller_set_selected(target, val, LV_ANIM_ON);
  if (id == _UI_ROLLER_PROPERTY_SELECTED)
    lv_roller_set_selected(target, val, LV_ANIM_OFF);
}

void _ui_slider_set_property(lv_obj_t *target, int id, int val) {
  if (id == _UI_SLIDER_PROPERTY_VALUE_WITH_ANIM)
    lv_slider_set_value(target, val, LV_ANIM_ON);
  if (id == _UI_SLIDER_PROPERTY_VALUE)
    lv_slider_set_value(target, val, LV_ANIM_OFF);
}

void _ui_screen_change(lv_obj_t *target, lv_scr_load_anim_t fademode, int spd, int delay) {
  lv_scr_load_anim(target, fademode, spd, delay, false);
}

void _ui_arc_increment(lv_obj_t *target, int val) {
  int old = lv_arc_get_value(target);
  lv_arc_set_value(target, old + val);
}

void _ui_bar_increment(lv_obj_t *target, int val, int anm) {
  int old = lv_bar_get_value(target);
  lv_bar_set_value(target, old + val, anm);
}

void _ui_slider_increment(lv_obj_t *target, int val, int anm) {
  int old = lv_slider_get_value(target);
  lv_slider_set_value(target, old + val, anm);
}

void _ui_flag_modify(lv_obj_t *target, int32_t flag, int value) {
  if (value == _UI_MODIFY_FLAG_TOGGLE) {
    if (lv_obj_has_flag(target, flag))
      lv_obj_clear_flag(target, flag);
    else
      lv_obj_add_flag(target, flag);
  } else if (value == _UI_MODIFY_FLAG_ADD)
    lv_obj_add_flag(target, flag);
  else
    lv_obj_clear_flag(target, flag);
}
void _ui_state_modify(lv_obj_t *target, int32_t state, int value) {
  if (value == _UI_MODIFY_STATE_TOGGLE) {
    if (lv_obj_has_state(target, state))
      lv_obj_clear_state(target, state);
    else
      lv_obj_add_state(target, state);
  } else if (value == _UI_MODIFY_STATE_ADD)
    lv_obj_add_state(target, state);
  else
    lv_obj_clear_state(target, state);
}

void _ui_opacity_set(lv_obj_t *target, int val) {
  lv_obj_set_style_opa(target, val, 0);
}

void _ui_anim_callback_set_x(lv_anim_t *a, int32_t v) {
  lv_obj_set_x((lv_obj_t *)a->user_data, v);
}

void _ui_anim_callback_set_y(lv_anim_t *a, int32_t v) {
  lv_obj_set_y((lv_obj_t *)a->user_data, v);
}

void _ui_anim_callback_set_width(lv_anim_t *a, int32_t v) {
  lv_obj_set_width((lv_obj_t *)a->user_data, v);
}

void _ui_anim_callback_set_height(lv_anim_t *a, int32_t v) {
  lv_obj_set_height((lv_obj_t *)a->user_data, v);
}

void _ui_anim_callback_set_opacity(lv_anim_t *a, int32_t v) {
  lv_obj_set_style_opa((lv_obj_t *)a->user_data, v, 0);
}

void _ui_anim_callback_set_image_zoom(lv_anim_t *a, int32_t v) {
  lv_img_set_zoom((lv_obj_t *)a->user_data, v);
}

void _ui_anim_callback_set_image_angle(lv_anim_t *a, int32_t v) {
  lv_img_set_angle((lv_obj_t *)a->user_data, v);
}

int32_t _ui_anim_callback_get_x(lv_anim_t *a) {
  return lv_obj_get_x_aligned((lv_obj_t *)a->user_data);
}

int32_t _ui_anim_callback_get_y(lv_anim_t *a) {
  return lv_obj_get_y_aligned((lv_obj_t *)a->user_data);
}

int32_t _ui_anim_callback_get_width(lv_anim_t *a) {
  return lv_obj_get_width((lv_obj_t *)a->user_data);
}

int32_t _ui_anim_callback_get_height(lv_anim_t *a) {
  return lv_obj_get_height((lv_obj_t *)a->user_data);
}

int32_t _ui_anim_callback_get_opacity(lv_anim_t *a) {
  return lv_obj_get_style_opa((lv_obj_t *)a->user_data, 0);
}

int32_t _ui_anim_callback_get_image_zoom(lv_anim_t *a) {
  return lv_img_get_zoom((lv_obj_t *)a->user_data);
}

int32_t _ui_anim_callback_get_image_angle(lv_anim_t *a) {
  return lv_img_get_angle((lv_obj_t *)a->user_data);
}

void _ui_arc_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *prefix, char *postfix) {
  char buf[_UI_TEMPORARY_STRING_BUFFER_SIZE];
  lv_snprintf(buf, sizeof(buf), "%s%d%s", prefix, (int)lv_arc_get_value(src), postfix);
  lv_label_set_text(trg, buf);
}

void _ui_slider_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *prefix, char *postfix) {
  char buf[_UI_TEMPORARY_STRING_BUFFER_SIZE];
  lv_snprintf(buf, sizeof(buf), "%s%d%s", prefix, (int)lv_slider_get_value(src), postfix);
  lv_label_set_text(trg, buf);
}

void _ui_checked_set_text_value(lv_obj_t *trg, lv_obj_t *src, char *txt_on, char *txt_off) {
  if (lv_obj_has_state(src, LV_STATE_CHECKED))
    lv_label_set_text(trg, txt_on);
  else
    lv_label_set_text(trg, txt_off);
}

static void msgbox_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *msgbox = lv_event_get_current_target(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_msgbox_close(msgbox);
  }
}

void warnning_msgbox(char *message) {
  static const char *btns[] = { "Close", "" };

  lv_obj_t *warn_mbox = lv_msgbox_create(NULL, "Warnning", message, btns, true);
  lv_obj_add_event_cb(warn_mbox, msgbox_event_cb, LV_EVENT_ALL, NULL);
  lv_obj_center(warn_mbox);
}

void enable_buttons(void) {
  /* Enable button */
  _ui_state_modify(ui_StartButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_StopButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_SettingButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_LogButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_REMOVE);
}

void disable_buttons(void) {
  _ui_state_modify(ui_StartButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_ADD);
  _ui_state_modify(ui_StopButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_ADD);
  _ui_state_modify(ui_SettingButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_ADD);
  _ui_state_modify(ui_LogButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_ADD);
}

void enable_start_button(void) {
  _ui_state_modify(ui_StartButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_REMOVE);
}

void disable_start_button(void) {
  _ui_state_modify(ui_StartButton, LV_STATE_DISABLED, _UI_MODIFY_STATE_ADD);
}

lv_obj_t *get_zone_panel_obj(ZONE zone) {
  if (zone >= 1 && zone <= 6) {
    lv_obj_t *zone_panel[6] = {
      ui_ZonePanel1, ui_ZonePanel2, ui_ZonePanel3, ui_ZonePanel4, ui_ZonePanel5, ui_ZonePanel6
    };
    return zone_panel[zone - 1];
  }
  return (lv_obj_t *)NULL;
}

lv_obj_t *get_zone_status_obj(ZONE zone) {
  if (zone >= 1 && zone <= 6) {
    lv_obj_t *zone_status[6] = { ui_ZoneStatus1, ui_ZoneStatus2, ui_ZoneStatus3,
                                 ui_ZoneStatus4, ui_ZoneStatus5, ui_ZoneStatus6 };
    return zone_status[zone - 1];
  }
  return (lv_obj_t *)NULL;
}

lv_obj_t *get_zone_num_obj(ZONE zone) {
  if (zone >= 1 && zone <= 6) {
    lv_obj_t *zone_num[6] = { ui_ZoneNum1, ui_ZoneNum2, ui_ZoneNum3, ui_ZoneNum4, ui_ZoneNum5, ui_ZoneNum6 };
    return zone_num[zone - 1];
  }
  return (lv_obj_t *)NULL;
}

lv_obj_t *get_zone_flow_meter_obj(ZONE zone) {
  if (zone >= 1 && zone <= 6) {
    lv_obj_t *zone_flow_meter[6] = { ui_ZoneFlowmeter1, ui_ZoneFlowmeter2, ui_ZoneFlowmeter3,
                                     ui_ZoneFlowmeter4, ui_ZoneFlowmeter5, ui_ZoneFlowmeter6 };
    return zone_flow_meter[zone - 1];
  }
  return (lv_obj_t *)NULL;
}

char *get_checked_zones(void) {
  int i, pos = 0;
  static char zones[20] = { 0 };

  lv_obj_t *zone_check_ui[6] = { ui_Zone1, ui_Zone2, ui_Zone3, ui_Zone4, ui_Zone5, ui_Zone6 };

  for (i = 0; i < 6; i++) {
    if (lv_obj_has_state(zone_check_ui[i], LV_STATE_CHECKED)) {
      pos += snprintf(&zones[pos], sizeof(zones), "%d,", i + 1);
    }
  }
  pos = strlen(zones);
  if (pos) {
    zones[pos - 1] = '\0';
  }

  return zones;
}

void set_zone_status(ZONE zone, bool start) {
  lv_obj_t *zone_status = get_zone_status_obj(zone);
  if (zone_status) {
    if (start) {
      lv_label_set_text(zone_status, "start");
      lv_obj_set_style_bg_color(zone_status, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      lv_label_set_text(zone_status, "stop");
      lv_obj_set_style_bg_color(zone_status, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}

void set_zone_number(ZONE zone, bool start) {
  lv_obj_t *zone_number = get_zone_num_obj(zone);
  if (zone_number) {
    if (start) {
      lv_obj_set_style_bg_color(zone_number, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      lv_obj_set_style_bg_color(zone_number, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}

void syscfg_set_flow_value(ZONE zone, char *flow_value) {
  int syscfg_idx[6] = { SYSCFG_I_ZONE1_FLOW, SYSCFG_I_ZONE2_FLOW, SYSCFG_I_ZONE3_FLOW,
                        SYSCFG_I_ZONE4_FLOW, SYSCFG_I_ZONE5_FLOW, SYSCFG_I_ZONE6_FLOW };
  char *syscfg_name[6] = { SYSCFG_N_ZONE1_FLOW, SYSCFG_N_ZONE2_FLOW, SYSCFG_N_ZONE3_FLOW,
                           SYSCFG_N_ZONE4_FLOW, SYSCFG_N_ZONE5_FLOW, SYSCFG_N_ZONE6_FLOW };

  if (zone >= 1 && zone <= 6) {
    syscfg_set(syscfg_idx[zone - 1], syscfg_name[zone - 1], flow_value);
  }
}

void syscfg_get_flow_value(ZONE zone, char *flow_value, size_t value_len) {
  int syscfg_idx[6] = { SYSCFG_I_ZONE1_FLOW, SYSCFG_I_ZONE2_FLOW, SYSCFG_I_ZONE3_FLOW,
                        SYSCFG_I_ZONE4_FLOW, SYSCFG_I_ZONE5_FLOW, SYSCFG_I_ZONE6_FLOW };
  char *syscfg_name[6] = { SYSCFG_N_ZONE1_FLOW, SYSCFG_N_ZONE2_FLOW, SYSCFG_N_ZONE3_FLOW,
                           SYSCFG_N_ZONE4_FLOW, SYSCFG_N_ZONE5_FLOW, SYSCFG_N_ZONE6_FLOW };

  if (zone >= 1 && zone <= 6) {
    syscfg_get(syscfg_idx[zone - 1], syscfg_name[zone - 1], flow_value, value_len);
    if (flow_value[0] == 0) {
      flow_value[0] = 0x30;
    }
  }
}

void set_zone_flow_value(ZONE zone, int flow_value) {
  char flow[20] = { 0 };
  int total_flow_value = 0;

  lv_obj_t *zone_flow = get_zone_flow_meter_obj(zone);
  if (zone_flow) {
    const char *curr_flow_value = lv_label_get_text(get_zone_flow_meter_obj(zone));
    total_flow_value = atoi(curr_flow_value) + flow_value;
    snprintf(flow, sizeof(flow), "%d", total_flow_value);
    lv_label_set_text(zone_flow, flow);
    syscfg_set_flow_value(zone, flow);
  }
}

void reset_settings(void) {
  lv_textarea_set_text(ui_FlowRateText, "");
  lv_textarea_set_text(ui_TimeHourText, "");
  lv_textarea_set_text(ui_TimeMinuteText, "");
  _ui_state_modify(ui_Zone1, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_Zone2, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_Zone3, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_Zone4, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_Zone5, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
  _ui_state_modify(ui_Zone6, LV_STATE_CHECKED, _UI_MODIFY_STATE_REMOVE);
}

void add_operation_list(const char *op_msg) {
  lv_textarea_add_text(ui_OperationList, op_msg);
}

void enable_zone(ZONE zone) {
  lv_obj_set_style_bg_color(get_zone_panel_obj(zone), lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_num_obj(zone), lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_status_obj(zone), lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_flow_meter_obj(zone), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void disable_zone(ZONE zone) {
  // modify the state as disable
  // set bgcolor as 0xe2e2e2 (light-gray)

  lv_obj_set_style_bg_color(get_zone_panel_obj(zone), lv_color_hex(0xE2E2E2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_num_obj(zone), lv_color_hex(0xE2E2E2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_status_obj(zone), lv_color_hex(0xE2E2E2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(get_zone_flow_meter_obj(zone), lv_color_hex(0xE2E2E2), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void update_zones(void) {
  char *cfg_name[6] = { SYSCFG_N_CHILD1_MAC, SYSCFG_N_CHILD2_MAC, SYSCFG_N_CHILD3_MAC,
                        SYSCFG_N_CHILD4_MAC, SYSCFG_N_CHILD5_MAC, SYSCFG_N_CHILD6_MAC };
  char mac_addr[13] = { 0 };

  for (int i = 0; i < 6; i++) {
    memset(mac_addr, 0x00, sizeof(mac_addr));
    if (syscfg_get(MFG_DATA, cfg_name[i], mac_addr, sizeof(mac_addr)) != 0) {
      disable_zone((ZONE)i + 1);
    } else {
      enable_zone((ZONE)i + 1);
    }
  }
}

void get_removed_device_list(char *device_list, size_t buf_len) {
  char *cfg_name[7] = { SYSCFG_N_MASTER_MAC, SYSCFG_N_CHILD1_MAC, SYSCFG_N_CHILD2_MAC, SYSCFG_N_CHILD3_MAC,
                        SYSCFG_N_CHILD4_MAC, SYSCFG_N_CHILD5_MAC, SYSCFG_N_CHILD6_MAC };
  char mac_addr[13] = { 0 };
  int pos = 0;

  device_list[0] = '\0';

  for (int i = 0; i < 7; i++) {
    memset(mac_addr, 0x00, sizeof(mac_addr));
    if (syscfg_get(MFG_DATA, cfg_name[i], mac_addr, sizeof(mac_addr)) != 0) {
      switch (i) {
        case 0: pos += snprintf(device_list + pos, buf_len, "%s\n", "Master"); break;
        case 1: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child1"); break;
        case 2: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child2"); break;
        case 3: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child3"); break;
        case 4: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child4"); break;
        case 5: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child5"); break;
        case 6: pos += snprintf(device_list + pos, buf_len, "%s\n", "Child6"); break;
        default: break;
      }
    }
  }
  device_list[pos - 1] = '\0';
}
