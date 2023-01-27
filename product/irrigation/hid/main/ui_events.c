// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <string.h>
#include <stdio.h>

#include "ui_helpers.h"

#include "espnow.h"
#include "syslog.h"
#include "syscfg.h"
#include "sys_config.h"
#include "sys_status.h"
#include "hid_config.h"
#include "command.h"

static const char* TAG = "UI_EVENT";

void OnStartEvent(lv_event_t* e) {
  config_value_t cfg = { 0 };
  stage_t stage = NONE_STAGE;

  // Read config value
  if (read_hid_config(&cfg)) {
    // Check if the configuration files are valid or not.
    // Start time should be included in irrigation available time.

    for (int i = 0; i < cfg.zone_cnt; i++) {
      LOGI(TAG, "zones = %d", cfg.zones[i]);
      LOGI(TAG, "flow = %d", cfg.flow_rate[i]);
    }
    show_timestamp(cfg.start_time);

    // validation check for start time : two time zone can be available for irrigation
    // device wakeup time : 5 am ~ 10 am (morning time), 5pm ~ 9pm (night time)
    // 05:00 ~ 10:00, 17:00 ~ 21:00 (0 ~ 24 hour)
    // device deep sleep time : 10:10 am ~ 04:50pm, 09:10 pm ~ 04:50 am
    // irrigation available time zones : 05:00 ~ 10:00 , 17:00 ~ 21:00
    if (!validation_of_start_irrigation_time(cfg.start_time, &stage)) {
      LOGW(TAG, "start_time is not available, please correct start time!!!");
      syscfg_unset(CFG_DATA, "hid_config");
      warnning_msgbox("start time is not available, please correct start time");
      return;
    }

    if (stage == CURR_STAGE || stage == NEXT_STAGE) {
      // Send configuration data to the main controller via ESP-NOW
      send_command_data(SET_CONFIG_COMMAND, &cfg, sizeof(config_value_t));
      LOGI(TAG, "Success to send Start command!!!");
      // Reset configuration data that is available in this current time,
      // if conf data will be available in next irrigation time, it should be keep and will send command in next time.
      syscfg_unset(CFG_DATA, "hid_config");
      // Set start irrigation flag
      set_start_irrigation(1);
    }
  } else {
    LOGE(TAG, "Failed to send Start command!!!");
    warnning_msgbox("There is no setting data");
  }
}

void OnStopEvent(lv_event_t* e) {
  send_command_data(STOP_COMMAND, NULL, 0);
  set_stop_irrigation(1);
}

void OnSettingEvent(lv_event_t* e) {
  reset_settings();
}

void OnResetEvent(lv_event_t* e) {}

void OnFlowRateEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_FlowRateText);
  }
}

void OnTimeHourEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_TimeHourText);
  }
}

void OnTimeMinuteEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_TimeMinuteText);
  }
}

void OnSettingSaveEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    const char* flow = lv_textarea_get_text(ui_FlowRateText);
    const char* zones = (const char*)get_checked_zones();
    const char* start_time_hour = lv_textarea_get_text(ui_TimeHourText);
    const char* start_time_minute = lv_textarea_get_text(ui_TimeMinuteText);
    if (save_hid_config(flow, start_time_hour, start_time_minute, zones)) {
      LOGI(TAG, "Success to save the configuration!!!");
    }
  }
}

void DeviceManagementEvent(char* type, lv_event_t* e) {
  int k = 0;
  char mac[16] = { 0 };
  char list_buf[200] = { 0 };
  LOGI(TAG, "type : %s", type);
  if (strncmp(type, "Master", 6) == 0) {
    syscfg_get(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC, mac, sizeof(mac));
    snprintf(list_buf, sizeof(mac) + 1, "%s\n", mac);
    lv_roller_set_options(ui_DM_Roller, list_buf, LV_ROLLER_MODE_NORMAL);
  } else if (strncmp(type, "Child", 5) == 0) {
    syscfg_get(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    syscfg_get(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    syscfg_get(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    syscfg_get(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    syscfg_get(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    syscfg_get(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC, mac, sizeof(mac));
    k += snprintf(list_buf + k, sizeof(mac) + 1, "%s\n", mac);

    lv_roller_set_options(ui_DM_Roller, list_buf, LV_ROLLER_MODE_NORMAL);

  } else {
    LOGI(TAG, "Error!!");
  }

  // Your code here
}

void DM_roller_event(char* type, lv_event_t* e) {
  char mac[16] = { 0 };
  LOGI(TAG, "roller_select : %s", type);

  syscfg_get(SYSCFG_I_MASTER_MAC, SYSCFG_N_MASTER_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Master");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD1_MAC, SYSCFG_N_CHILD1_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_1");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD2_MAC, SYSCFG_N_CHILD2_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_2");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD3_MAC, SYSCFG_N_CHILD3_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_3");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD4_MAC, SYSCFG_N_CHILD4_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_4");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD5_MAC, SYSCFG_N_CHILD5_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_5");
    return;
  }

  syscfg_get(SYSCFG_I_CHILD6_MAC, SYSCFG_N_CHILD6_MAC, mac, sizeof(mac));
  if (strncmp(type, mac, sizeof(mac)) == 0) {
    lv_label_set_text(ui_DM_Roller_Label, "Child_6");
    return;
  }
}
