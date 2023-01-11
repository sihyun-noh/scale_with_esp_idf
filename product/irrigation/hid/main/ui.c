// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_Main;
lv_obj_t *ui_MainStatusPanel;
lv_obj_t *ui_Zone1;
lv_obj_t *ui_ZoneStatus1;
lv_obj_t *ui_ZoneFlowmetar1;
lv_obj_t *ui_ZoneNum1;
lv_obj_t *ui_Zone2;
lv_obj_t *ui_ZoneStatus2;
lv_obj_t *ui_ZoneFlowmetar2;
lv_obj_t *ui_ZoneNum2;
lv_obj_t *ui_Zone3;
lv_obj_t *ui_ZoneStatus3;
lv_obj_t *ui_ZoneFlowmetar3;
lv_obj_t *ui_ZoneNum3;
lv_obj_t *ui_Zone4;
lv_obj_t *ui_ZoneStatus4;
lv_obj_t *ui_ZoneFlowmetar4;
lv_obj_t *ui_ZoneNum4;
lv_obj_t *ui_Zone5;
lv_obj_t *ui_ZoneStatus5;
lv_obj_t *ui_ZoneFlowmetar5;
lv_obj_t *ui_ZoneNum5;
lv_obj_t *ui_Zone6;
lv_obj_t *ui_ZoneStatus6;
lv_obj_t *ui_ZoneFlowmetar6;
lv_obj_t *ui_ZoneNum6;
void ui_event_StartButton(lv_event_t *e);
lv_obj_t *ui_StartButton;
lv_obj_t *ui_StartButtonLabel;
void ui_event_StopButton(lv_event_t *e);
lv_obj_t *ui_StopButton;
lv_obj_t *ui_StopButtonLabel;
void ui_event_ResetButton(lv_event_t *e);
lv_obj_t *ui_ResetButton;
lv_obj_t *ui_ResetButtonLabel;
void ui_event_SettingButton(lv_event_t *e);
lv_obj_t *ui_SettingButton;
lv_obj_t *ui_SettingButtonLabel;
lv_obj_t *ui_OperationListPanel;
lv_obj_t *ui_Setting;
lv_obj_t *ui_SettingPanel;
lv_obj_t *ui_SettingKeyboard;
lv_obj_t *ui_SettingTitle;
void ui_event_FlowRateText(lv_event_t *e);
lv_obj_t *ui_FlowRateText;
void ui_event_ZoneAreaText(lv_event_t *e);
lv_obj_t *ui_ZoneAreaText;
void ui_event_TimeHourText(lv_event_t *e);
lv_obj_t *ui_TimeHourText;
lv_obj_t *ui_TimeSeperate;
void ui_event_TimeMinuteText(lv_event_t *e);
lv_obj_t *ui_TimeMinuteText;
void ui_event_SettingSaveButton(lv_event_t *e);
lv_obj_t *ui_SettingSaveButton;
lv_obj_t *ui_SettingSaveButtonLabel;
void ui_event_SettingCancelButton(lv_event_t *e);
lv_obj_t *ui_SettingCancelButton;
lv_obj_t *ui_SettingCancelButtonLabel;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_StartButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnStartEvent(e);
  }
}
void ui_event_StopButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnStopEvent(e);
  }
}
void ui_event_ResetButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnResetEvent(e);
  }
}
void ui_event_SettingButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Setting, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
  }
}
void ui_event_FlowRateText(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnFlowRateEvent(e);
  }
}
void ui_event_ZoneAreaText(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnZoneAreaEvent(e);
  }
}
void ui_event_TimeHourText(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnTimeHourEvent(e);
  }
}
void ui_event_TimeMinuteText(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnTimeMinuteEvent(e);
  }
}
void ui_event_SettingSaveButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnSettingSaveEvent(e);
    _ui_screen_change(ui_Main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
  }
}
void ui_event_SettingCancelButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
  }
}

///////////////////// SCREENS ////////////////////
void ui_Main_screen_init(void) {
  ui_Main = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Main, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Main, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Main, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_MainStatusPanel = lv_obj_create(ui_Main);
  lv_obj_set_width(ui_MainStatusPanel, 452);
  lv_obj_set_height(ui_MainStatusPanel, 117);
  lv_obj_set_x(ui_MainStatusPanel, 0);
  lv_obj_set_y(ui_MainStatusPanel, -87);
  lv_obj_set_align(ui_MainStatusPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_MainStatusPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_MainStatusPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone1 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone1, lv_pct(15));
  lv_obj_set_height(ui_Zone1, lv_pct(120));
  lv_obj_set_x(ui_Zone1, lv_pct(-3));
  lv_obj_set_y(ui_Zone1, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone1, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone1, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus1 = lv_label_create(ui_Zone1);
  lv_obj_set_width(ui_ZoneStatus1, 55);
  lv_obj_set_height(ui_ZoneStatus1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus1, 0);
  lv_obj_set_y(ui_ZoneStatus1, -10);
  lv_obj_set_align(ui_ZoneStatus1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus1, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus1, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus1, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar1 = lv_label_create(ui_Zone1);
  lv_obj_set_width(ui_ZoneFlowmetar1, 55);
  lv_obj_set_height(ui_ZoneFlowmetar1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar1, 0);
  lv_obj_set_y(ui_ZoneFlowmetar1, 55);
  lv_obj_set_align(ui_ZoneFlowmetar1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar1, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum1 = lv_label_create(ui_Zone1);
  lv_obj_set_width(ui_ZoneNum1, 55);
  lv_obj_set_height(ui_ZoneNum1, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum1, 0);
  lv_obj_set_y(ui_ZoneNum1, 13);
  lv_obj_set_align(ui_ZoneNum1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum1, "1");
  lv_obj_set_style_text_align(ui_ZoneNum1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum1, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone2 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone2, lv_pct(15));
  lv_obj_set_height(ui_Zone2, lv_pct(120));
  lv_obj_set_x(ui_Zone2, lv_pct(15));
  lv_obj_set_y(ui_Zone2, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone2, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone2, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus2 = lv_label_create(ui_Zone2);
  lv_obj_set_width(ui_ZoneStatus2, 55);
  lv_obj_set_height(ui_ZoneStatus2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus2, 0);
  lv_obj_set_y(ui_ZoneStatus2, -10);
  lv_obj_set_align(ui_ZoneStatus2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus2, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus2, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus2, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar2 = lv_label_create(ui_Zone2);
  lv_obj_set_width(ui_ZoneFlowmetar2, 55);
  lv_obj_set_height(ui_ZoneFlowmetar2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar2, 0);
  lv_obj_set_y(ui_ZoneFlowmetar2, 55);
  lv_obj_set_align(ui_ZoneFlowmetar2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar2, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum2 = lv_label_create(ui_Zone2);
  lv_obj_set_width(ui_ZoneNum2, 55);
  lv_obj_set_height(ui_ZoneNum2, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum2, 0);
  lv_obj_set_y(ui_ZoneNum2, 13);
  lv_obj_set_align(ui_ZoneNum2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum2, "2");
  lv_obj_set_style_text_align(ui_ZoneNum2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum2, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum2, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone3 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone3, lv_pct(15));
  lv_obj_set_height(ui_Zone3, lv_pct(120));
  lv_obj_set_x(ui_Zone3, lv_pct(33));
  lv_obj_set_y(ui_Zone3, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone3, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone3, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus3 = lv_label_create(ui_Zone3);
  lv_obj_set_width(ui_ZoneStatus3, 55);
  lv_obj_set_height(ui_ZoneStatus3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus3, 0);
  lv_obj_set_y(ui_ZoneStatus3, -10);
  lv_obj_set_align(ui_ZoneStatus3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus3, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus3, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus3, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar3 = lv_label_create(ui_Zone3);
  lv_obj_set_width(ui_ZoneFlowmetar3, 55);
  lv_obj_set_height(ui_ZoneFlowmetar3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar3, 0);
  lv_obj_set_y(ui_ZoneFlowmetar3, 55);
  lv_obj_set_align(ui_ZoneFlowmetar3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar3, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum3 = lv_label_create(ui_Zone3);
  lv_obj_set_width(ui_ZoneNum3, 55);
  lv_obj_set_height(ui_ZoneNum3, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum3, 0);
  lv_obj_set_y(ui_ZoneNum3, 13);
  lv_obj_set_align(ui_ZoneNum3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum3, "3");
  lv_obj_set_style_text_align(ui_ZoneNum3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum3, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum3, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum3, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone4 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone4, lv_pct(15));
  lv_obj_set_height(ui_Zone4, lv_pct(120));
  lv_obj_set_x(ui_Zone4, lv_pct(51));
  lv_obj_set_y(ui_Zone4, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone4, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone4, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus4 = lv_label_create(ui_Zone4);
  lv_obj_set_width(ui_ZoneStatus4, 55);
  lv_obj_set_height(ui_ZoneStatus4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus4, 0);
  lv_obj_set_y(ui_ZoneStatus4, -10);
  lv_obj_set_align(ui_ZoneStatus4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus4, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus4, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus4, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar4 = lv_label_create(ui_Zone4);
  lv_obj_set_width(ui_ZoneFlowmetar4, 55);
  lv_obj_set_height(ui_ZoneFlowmetar4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar4, 0);
  lv_obj_set_y(ui_ZoneFlowmetar4, 55);
  lv_obj_set_align(ui_ZoneFlowmetar4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar4, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum4 = lv_label_create(ui_Zone4);
  lv_obj_set_width(ui_ZoneNum4, 55);
  lv_obj_set_height(ui_ZoneNum4, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum4, 0);
  lv_obj_set_y(ui_ZoneNum4, 13);
  lv_obj_set_align(ui_ZoneNum4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum4, "4");
  lv_obj_set_style_text_align(ui_ZoneNum4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum4, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum4, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum4, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone5 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone5, lv_pct(15));
  lv_obj_set_height(ui_Zone5, lv_pct(120));
  lv_obj_set_x(ui_Zone5, lv_pct(69));
  lv_obj_set_y(ui_Zone5, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone5, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone5, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus5 = lv_label_create(ui_Zone5);
  lv_obj_set_width(ui_ZoneStatus5, 55);
  lv_obj_set_height(ui_ZoneStatus5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus5, 0);
  lv_obj_set_y(ui_ZoneStatus5, -10);
  lv_obj_set_align(ui_ZoneStatus5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus5, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus5, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus5, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar5 = lv_label_create(ui_Zone5);
  lv_obj_set_width(ui_ZoneFlowmetar5, 55);
  lv_obj_set_height(ui_ZoneFlowmetar5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar5, 0);
  lv_obj_set_y(ui_ZoneFlowmetar5, 55);
  lv_obj_set_align(ui_ZoneFlowmetar5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar5, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar5, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum5 = lv_label_create(ui_Zone5);
  lv_obj_set_width(ui_ZoneNum5, 55);
  lv_obj_set_height(ui_ZoneNum5, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum5, 0);
  lv_obj_set_y(ui_ZoneNum5, 13);
  lv_obj_set_align(ui_ZoneNum5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum5, "5");
  lv_obj_set_style_text_align(ui_ZoneNum5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum5, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum5, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum5, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Zone6 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_Zone6, lv_pct(15));
  lv_obj_set_height(ui_Zone6, lv_pct(120));
  lv_obj_set_x(ui_Zone6, lv_pct(87));
  lv_obj_set_y(ui_Zone6, lv_pct(-10));
  lv_obj_clear_flag(ui_Zone6, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Zone6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Zone6, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Zone6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus6 = lv_label_create(ui_Zone6);
  lv_obj_set_width(ui_ZoneStatus6, 55);
  lv_obj_set_height(ui_ZoneStatus6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus6, 0);
  lv_obj_set_y(ui_ZoneStatus6, -10);
  lv_obj_set_align(ui_ZoneStatus6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus6, "start");
  lv_obj_set_style_text_align(ui_ZoneStatus6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus6, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus6, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmetar6 = lv_label_create(ui_Zone6);
  lv_obj_set_width(ui_ZoneFlowmetar6, 55);
  lv_obj_set_height(ui_ZoneFlowmetar6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmetar6, 0);
  lv_obj_set_y(ui_ZoneFlowmetar6, 55);
  lv_obj_set_align(ui_ZoneFlowmetar6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmetar6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmetar6, "600");
  lv_obj_set_style_text_align(ui_ZoneFlowmetar6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmetar6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmetar6, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmetar6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmetar6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmetar6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmetar6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmetar6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum6 = lv_label_create(ui_Zone6);
  lv_obj_set_width(ui_ZoneNum6, 55);
  lv_obj_set_height(ui_ZoneNum6, lv_pct(60));
  lv_obj_set_x(ui_ZoneNum6, 0);
  lv_obj_set_y(ui_ZoneNum6, 13);
  lv_obj_set_align(ui_ZoneNum6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneNum6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneNum6, "6");
  lv_obj_set_style_text_align(ui_ZoneNum6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ZoneNum6, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneNum6, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum6, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_StartButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_StartButton, 101);
  lv_obj_set_height(ui_StartButton, 47);
  lv_obj_set_x(ui_StartButton, -170);
  lv_obj_set_y(ui_StartButton, 17);
  lv_obj_set_align(ui_StartButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_StartButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_StartButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_StartButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_StartButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_StartButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  /* Disable button */
  lv_obj_add_state(ui_StartButton, LV_STATE_DISABLED);

  ui_StartButtonLabel = lv_label_create(ui_StartButton);
  lv_obj_set_width(ui_StartButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_StartButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_StartButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_StartButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_StartButtonLabel, "Start");
  lv_obj_set_style_text_color(ui_StartButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_StartButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_StartButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_StopButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_StopButton, 96);
  lv_obj_set_height(ui_StopButton, 49);
  lv_obj_set_x(ui_StopButton, -57);
  lv_obj_set_y(ui_StopButton, 17);
  lv_obj_set_align(ui_StopButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_StopButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_StopButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_StopButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_StopButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_StopButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  /* Disable button */
  lv_obj_add_state(ui_StopButton, LV_STATE_DISABLED);

  ui_StopButtonLabel = lv_label_create(ui_StopButton);
  lv_obj_set_width(ui_StopButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_StopButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_StopButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_StopButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_StopButtonLabel, "Stop");
  lv_obj_set_style_text_color(ui_StopButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_StopButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_StopButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ResetButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_ResetButton, 88);
  lv_obj_set_height(ui_ResetButton, 48);
  lv_obj_set_x(ui_ResetButton, 178);
  lv_obj_set_y(ui_ResetButton, 19);
  lv_obj_set_align(ui_ResetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_ResetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_ResetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_ResetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ResetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ResetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  /* Disable button */
  lv_obj_add_state(ui_ResetButton, LV_STATE_DISABLED);

  ui_ResetButtonLabel = lv_label_create(ui_ResetButton);
  lv_obj_set_width(ui_ResetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_ResetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_ResetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_ResetButtonLabel, "Reset");
  lv_obj_set_style_text_color(ui_ResetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_ResetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ResetButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_SettingButton, 112);
  lv_obj_set_height(ui_SettingButton, 48);
  lv_obj_set_x(ui_SettingButton, 63);
  lv_obj_set_y(ui_SettingButton, 19);
  lv_obj_set_align(ui_SettingButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SettingButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SettingButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_SettingButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SettingButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SettingButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  /* Disable button */
  lv_obj_add_state(ui_SettingButton, LV_STATE_DISABLED);

  ui_SettingButtonLabel = lv_label_create(ui_SettingButton);
  lv_obj_set_width(ui_SettingButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingButtonLabel, -2);
  lv_obj_set_y(ui_SettingButtonLabel, -1);
  lv_obj_set_align(ui_SettingButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_SettingButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_SettingButtonLabel, "Setting");
  lv_obj_set_style_text_color(ui_SettingButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SettingButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_SettingButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_OperationListPanel = lv_obj_create(ui_Main);
  lv_obj_set_width(ui_OperationListPanel, 413);
  lv_obj_set_height(ui_OperationListPanel, 80);
  lv_obj_set_x(ui_OperationListPanel, -4);
  lv_obj_set_y(ui_OperationListPanel, 104);
  lv_obj_set_align(ui_OperationListPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_OperationListPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_OperationListPanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_StartButton, ui_event_StartButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_StopButton, ui_event_StopButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_ResetButton, ui_event_ResetButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingButton, ui_event_SettingButton, LV_EVENT_ALL, NULL);
}
void ui_Setting_screen_init(void) {
  ui_Setting = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Setting, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Setting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Setting, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Setting, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingPanel = lv_obj_create(ui_Setting);
  lv_obj_set_width(ui_SettingPanel, 459);
  lv_obj_set_height(ui_SettingPanel, 292);
  lv_obj_set_x(ui_SettingPanel, 0);
  lv_obj_set_y(ui_SettingPanel, -2);
  lv_obj_set_align(ui_SettingPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_SettingPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_SettingKeyboard = lv_keyboard_create(ui_SettingPanel);
  lv_keyboard_set_mode(ui_SettingKeyboard, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_set_width(ui_SettingKeyboard, 410);
  lv_obj_set_height(ui_SettingKeyboard, 180);
  lv_obj_set_x(ui_SettingKeyboard, 2);
  lv_obj_set_y(ui_SettingKeyboard, 47);
  lv_obj_set_align(ui_SettingKeyboard, LV_ALIGN_CENTER);

  ui_SettingTitle = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingTitle, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingTitle, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingTitle, -55);
  lv_obj_set_y(ui_SettingTitle, -125);
  lv_obj_set_align(ui_SettingTitle, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingTitle, "flow rate / Zone area / time set (Hour, Min)");

  ui_FlowRateText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_FlowRateText, 65);
  lv_obj_set_height(ui_FlowRateText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_FlowRateText, -176);
  lv_obj_set_y(ui_FlowRateText, -84);
  lv_obj_set_align(ui_FlowRateText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_FlowRateText, "Placeholder...");
  lv_textarea_set_one_line(ui_FlowRateText, true);

  ui_ZoneAreaText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_ZoneAreaText, 77);
  lv_obj_set_height(ui_ZoneAreaText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_ZoneAreaText, -96);
  lv_obj_set_y(ui_ZoneAreaText, -84);
  lv_obj_set_align(ui_ZoneAreaText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_ZoneAreaText, "Placeholder...");
  lv_textarea_set_one_line(ui_ZoneAreaText, true);

  ui_TimeHourText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeHourText, 58);
  lv_obj_set_height(ui_TimeHourText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_TimeHourText, -16);
  lv_obj_set_y(ui_TimeHourText, -84);
  lv_obj_set_align(ui_TimeHourText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeHourText, "Placeholder...");
  lv_textarea_set_one_line(ui_TimeHourText, true);

  ui_TimeSeperate = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_TimeSeperate, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_TimeSeperate, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_TimeSeperate, 25);
  lv_obj_set_y(ui_TimeSeperate, -83);
  lv_obj_set_align(ui_TimeSeperate, LV_ALIGN_CENTER);
  lv_label_set_text(ui_TimeSeperate, ":");

  ui_TimeMinuteText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeMinuteText, 58);
  lv_obj_set_height(ui_TimeMinuteText, 37);
  lv_obj_set_x(ui_TimeMinuteText, 65);
  lv_obj_set_y(ui_TimeMinuteText, -85);
  lv_obj_set_align(ui_TimeMinuteText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeMinuteText, "Placeholder...");

  ui_SettingSaveButton = lv_btn_create(ui_Setting);
  lv_obj_set_width(ui_SettingSaveButton, 70);
  lv_obj_set_height(ui_SettingSaveButton, 37);
  lv_obj_set_x(ui_SettingSaveButton, 162);
  lv_obj_set_y(ui_SettingSaveButton, -119);
  lv_obj_set_align(ui_SettingSaveButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SettingSaveButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SettingSaveButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_SettingSaveButton, lv_color_hex(0x4E4D4D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SettingSaveButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingSaveButtonLabel = lv_label_create(ui_SettingSaveButton);
  lv_obj_set_width(ui_SettingSaveButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingSaveButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_SettingSaveButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_SettingSaveButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_SettingSaveButtonLabel, "Save");

  ui_SettingCancelButton = lv_btn_create(ui_Setting);
  lv_obj_set_width(ui_SettingCancelButton, 70);
  lv_obj_set_height(ui_SettingCancelButton, 37);
  lv_obj_set_x(ui_SettingCancelButton, 162);
  lv_obj_set_y(ui_SettingCancelButton, -73);
  lv_obj_set_align(ui_SettingCancelButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SettingCancelButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SettingCancelButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags

  ui_SettingCancelButtonLabel = lv_label_create(ui_SettingCancelButton);
  lv_obj_set_width(ui_SettingCancelButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingCancelButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingCancelButtonLabel, -2);
  lv_obj_set_y(ui_SettingCancelButtonLabel, 0);
  lv_obj_set_align(ui_SettingCancelButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingCancelButtonLabel, "Cancel");

  lv_keyboard_set_textarea(ui_SettingKeyboard, ui_FlowRateText);
  lv_obj_add_event_cb(ui_FlowRateText, ui_event_FlowRateText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_ZoneAreaText, ui_event_ZoneAreaText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_TimeHourText, ui_event_TimeHourText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_TimeMinuteText, ui_event_TimeMinuteText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingSaveButton, ui_event_SettingSaveButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingCancelButton, ui_event_SettingCancelButton, LV_EVENT_ALL, NULL);
}

void ui_init(void) {
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                            false, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);
  ui_Main_screen_init();
  ui_Setting_screen_init();
  lv_disp_load_scr(ui_Main);
}
