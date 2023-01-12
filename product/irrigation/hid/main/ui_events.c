// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <string.h>

#include "comm_packet.h"
#include "ui_helpers.h"

#include "espnow.h"
#include "syslog.h"
#include "syscfg.h"
#include "hid_config.h"
#include "command.h"

static const char* TAG = "UI_EVENT";

void OnStartEvent(lv_event_t* e) {
  config_value_t cfg = { 0 };
  // irrigation_message_t msg = { 0 };

  // Read config value
  if (read_hid_config(&cfg)) {
    // Check if the configuration files are valid or not.
    // Start time should be included in irrigation available time.

    for (int i = 0; i < cfg.zone_cnt; i++) {
      LOGI(TAG, "zones = %d", cfg.zones[i]);
      LOGI(TAG, "flow = %d", cfg.flow_rate[i]);
    }
    show_timestamp(cfg.start_time);

    // Send configuration data to the main controller via ESP-NOW
#if 0
    msg.sender_type = SET_CONFIG;
    memcpy(&msg.config, &cfg, sizeof(config_value_t));
    espnow_send_data(espnow_get_master_addr(), (uint8_t*)&msg, sizeof(irrigation_message_t));
#endif
    send_command_data(SET_CONFIG_COMMAND, &cfg, sizeof(config_value_t));
    LOGI(TAG, "Success to send Start command!!!");
    // lv_label_set_text(ui_StartButtonLabel, "In-Progress");
    // Reset configuration data
    syscfg_unset(CFG_DATA, "hid_config");
  } else {
    LOGE(TAG, "Failed to send Start command!!!");
    warnning_msgbox("There is no setting data");
  }
}

void OnStopEvent(lv_event_t* e) {
  // Your code here
}

void OnResetEvent(lv_event_t* e) {
  // Your code here
}

void OnFlowRateEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_FlowRateText);
  }
}

void OnZoneAreaEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_SettingKeyboard, ui_ZoneAreaText);
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
    const char* zones = lv_textarea_get_text(ui_ZoneAreaText);
    const char* start_time_hour = lv_textarea_get_text(ui_TimeHourText);
    const char* start_time_minute = lv_textarea_get_text(ui_TimeMinuteText);
    if (save_hid_config(flow, start_time_hour, start_time_minute, zones)) {
      LOGI(TAG, "Success to save the configuration!!!");
    }
  }
}
