// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <string.h>

#include "ui.h"

#include "espnow.h"
#include "syslog.h"
#include "syscfg.h"
#include "hid_config.h"

static const char* TAG = "UI_EVENT";

extern uint8_t main_mac_addr[];

void OnStartEvent(lv_event_t* e) {
  // Your code here
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
    config_value_t cfg = { 0 };
    irrigation_message_t msg = { 0 };

    const char* flow = lv_textarea_get_text(ui_FlowRateText);
    const char* zones = lv_textarea_get_text(ui_ZoneAreaText);
    const char* start_time_hour = lv_textarea_get_text(ui_TimeHourText);
    const char* start_time_minute = lv_textarea_get_text(ui_TimeMinuteText);
    if (save_hid_config(flow, start_time_hour, start_time_minute, zones)) {
      // Read config value
      read_hid_config(&cfg);
      LOGI(TAG, "flow = %d", cfg.flow_rate);
      for (int i = 0; i < cfg.zone_cnt; i++) {
        LOGI(TAG, "zones = %d", cfg.zones[i]);
      }
      show_timestamp(cfg.start_time);

      // Send configuration data to the main controller via ESP-NOW
      msg.sender_type = SET_CONFIG;
      memcpy(&msg.config, &cfg, sizeof(config_value_t));
      espnow_send_data(main_mac_addr, (uint8_t*)&msg, sizeof(irrigation_message_t));
    }
  }
}
