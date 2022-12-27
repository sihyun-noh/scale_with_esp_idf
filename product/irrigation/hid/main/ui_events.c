// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include "ui.h"
#include "syslog.h"
#include "syscfg.h"
#include "hid_config.h"

#include <string.h>

#define TAG "UI_EVENT"

void Screen2SaveButtonEvent(lv_event_t *e) {
#if 0
  irrigation_message_t msg;
  memset(&msg, 0x00, sizeof(irrigation_message_t));

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    int flow_rate = 0;
    int i = 0, zone = 0;
    const char *flow = lv_textarea_get_text(ui_Screen2TextArea);
    const char *zones = lv_textarea_get_text(ui_Screen2TextArea1);
    const char *start_time = lv_textarea_get_text(ui_Screen2TextArea2);
    if (!flow || !start_time) {
      LOGW(TAG, "flow and start_time should be filled!!!");
      return;
    }
    flow_rate = atoi(flow);
    if (flow_rate <= 0) {
      LOGW(TAG, "flow rate cannot be 0 or negative");
    } else {
      msg.config.flow_rate = flow_rate;
    }
    LOGI(TAG, "flow data = %s, zones = %s, start_time = %s", flow, zones, start_time);
    if (zones) {
      char *beg = zones;
      char *pch = (char *)strpbrk(beg, ",");
      while (pch != NULL) {
        *pch = '\0';
        zone = atoi(beg);
        if (zone >= 1 && zone <= 6) {
          msg.config.zones[i++] = zone;
        }
        beg = pch + 1;
        pch = (char *)strpbrk(beg, ",");
      }
      if (beg) {
        msg.config.zones[i] = atoi(beg);
      }
      i = 0;
      while (msg.config.zones[i] != 0) {
        LOGI(TAG, "zones[%d] = %d", i, msg.config.zones[i]);
        i++;
      }
    }
    msg.sender_type = SET_CONFIG;
    syscfg_set_blob(CFG_DATA, "set_config", &msg, sizeof(irrigation_message_t));
  }
#endif

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    config_t cfg = { 0 };
    const char *flow = lv_textarea_get_text(ui_Screen2TextArea);
    const char *zones = lv_textarea_get_text(ui_Screen2TextArea1);
    const char *start_time = lv_textarea_get_text(ui_Screen2TextArea2);
    save_hid_config(flow, start_time, zones);

    // Read config value
    read_hid_config(&cfg);
    LOGI(TAG, "flow = %d", cfg.flow_rate);
    LOGI(TAG, "zones = %d", cfg.zones[0]);
    show_timestamp(cfg.start_time);
  }
}

void Screen2TextAreaEvent(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_Screen2_Keyboard, ui_Screen2TextArea);
  }
}

void Screen2TextArea1Event(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_Screen2_Keyboard, ui_Screen2TextArea1);
  }
}

void Screen2TextArea2Event(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_keyboard_set_textarea(ui_Screen2_Keyboard, ui_Screen2TextArea2);
  }
}
