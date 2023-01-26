// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <string.h>

#include "ui_helpers.h"

#include "espnow.h"
#include "syslog.h"
#include "syscfg.h"
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

    if (stage == CURR_STAGE) {
      // Send configuration data to the main controller via ESP-NOW
      send_command_data(SET_CONFIG_COMMAND, &cfg, sizeof(config_value_t));
      LOGI(TAG, "Success to send Start command!!!");
      // Reset configuration data that is available in this current time,
      // if conf data will be available in next irrigation time, it should be keep and will send command in next time.
      syscfg_unset(CFG_DATA, "hid_config");
      // Set start irrigation flag
      set_start_irrigation(1);
    } else if (stage == NEXT_STAGE) {
      LOGI(TAG, "Start command will be runned at next stage");
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
