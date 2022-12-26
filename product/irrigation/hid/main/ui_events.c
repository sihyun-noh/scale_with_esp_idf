// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include "ui.h"
#include "syslog.h"
#include "widgets/lv_textarea.h"

static const char *TAG = "UI_EVENT";

void Screen2SaveButtonEvent(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    const char *flow = lv_textarea_get_text(ui_Screen2TextArea);
    const char *zones = lv_textarea_get_text(ui_Screen2TextArea1);
    const char *start_time = lv_textarea_get_text(ui_Screen2TextArea2);
    LOGI(TAG, "flow data = %s, zones = %s, start_time = %s", flow, zones, start_time);
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
