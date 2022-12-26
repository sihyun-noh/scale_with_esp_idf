// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_Screen1;
lv_obj_t *ui_Screen1StatusPanel;
lv_obj_t *ui_Screen1Zone1;
lv_obj_t *ui_Screen1ZoneStatus1;
lv_obj_t *ui_Screen1ZoneFlowmetar1;
lv_obj_t *ui_Screen1ZoneNum1;
lv_obj_t *ui_Screen1Zone2;
lv_obj_t *ui_Screen1ZoneStatus2;
lv_obj_t *ui_Screen1ZoneFlowmetar2;
lv_obj_t *ui_Screen1ZoneNum2;
lv_obj_t *ui_Screen1Zone3;
lv_obj_t *ui_Screen1ZoneStatus3;
lv_obj_t *ui_Screen1ZoneFlowmetar3;
lv_obj_t *ui_Screen1ZoneNum3;
lv_obj_t *ui_Screen1Zone4;
lv_obj_t *ui_Screen1ZoneStatus4;
lv_obj_t *ui_Screen1ZoneFlowmetar4;
lv_obj_t *ui_Screen1ZoneNum4;
lv_obj_t *ui_Screen1Zone5;
lv_obj_t *ui_Screen1ZoneStatus5;
lv_obj_t *ui_Screen1ZoneFlowmetar5;
lv_obj_t *ui_Screen1ZoneNum5;
lv_obj_t *ui_Screen1Zone6;
lv_obj_t *ui_Screen1ZoneStatus6;
lv_obj_t *ui_Screen1ZoneFlowmetar6;
lv_obj_t *ui_Screen1ZoneNum6;
lv_obj_t *ui_ProductSetButton;
lv_obj_t *ui_ProductSetButtonLabel;
void ui_event_OperationSetButton(lv_event_t *e);
lv_obj_t *ui_OperationSetButton;
lv_obj_t *ui_OperationSetButtonLabel;
lv_obj_t *ui_FlowResetButton;
lv_obj_t *ui_FlowResetButtonLabel;
lv_obj_t *ui_SettingResetButton;
lv_obj_t *ui_SettingResetButtonLabel;
lv_obj_t *ui_Screen1FICLabel;
lv_obj_t *ui_Screen1TimeLabel;
lv_obj_t *ui_Screen1OperationListPanel;
lv_obj_t *ui_Screen1OperationListLabel1;
lv_obj_t *ui_Screen1OperationListLabel2;
lv_obj_t *ui_Screen1OperationListLabel3;
lv_obj_t *ui_Screen1OperationListLabel14;
lv_obj_t *ui_Screen1MasterStatusLabel;
lv_obj_t *ui_Screen2;
lv_obj_t *ui_Screen2MainPanel;
lv_obj_t *ui_Screen2_Keyboard;
lv_obj_t *ui_Screen2TextArea;
lv_obj_t *ui_Screen2TextArea1;
lv_obj_t *ui_Screen2TextArea2;
void ui_event_Screen2SaveButton(lv_event_t *e);
lv_obj_t *ui_Screen2SaveButton;
lv_obj_t *ui_Screen2SaveButtonLabel;
lv_obj_t *ui_Scree2ExampleLaBel1;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_OperationSetButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Screen2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
  }
}

void ui_event_Screen2TextArea(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    Screen2TextAreaEvent(e);
  }
}

void ui_event_Screen2TextArea1(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    Screen2TextArea1Event(e);
  }
}

void ui_event_Screen2TextArea2(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    Screen2TextArea2Event(e);
  }
}

void ui_event_Screen2SaveButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Screen1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 500, 0);
    Screen2SaveButtonEvent(e);
  }
}

///////////////////// SCREENS ////////////////////
void ui_Screen1_screen_init(void) {
  ui_Screen1 = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1StatusPanel = lv_obj_create(ui_Screen1);
  lv_obj_set_width(ui_Screen1StatusPanel, 452);
  lv_obj_set_height(ui_Screen1StatusPanel, 117);
  lv_obj_set_x(ui_Screen1StatusPanel, 0);
  lv_obj_set_y(ui_Screen1StatusPanel, -58);
  lv_obj_set_align(ui_Screen1StatusPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_Screen1StatusPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1StatusPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone1 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone1, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone1, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone1, lv_pct(-3));
  lv_obj_set_y(ui_Screen1Zone1, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone1, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone1, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus1 = lv_label_create(ui_Screen1Zone1);
  lv_obj_set_width(ui_Screen1ZoneStatus1, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus1, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus1, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus1, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus1, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus1, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar1 = lv_label_create(ui_Screen1Zone1);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar1, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar1, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar1, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar1, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum1 = lv_label_create(ui_Screen1Zone1);
  lv_obj_set_width(ui_Screen1ZoneNum1, 55);
  lv_obj_set_height(ui_Screen1ZoneNum1, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum1, 0);
  lv_obj_set_y(ui_Screen1ZoneNum1, 13);
  lv_obj_set_align(ui_Screen1ZoneNum1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum1, "1");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum1, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum1, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone2 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone2, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone2, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone2, lv_pct(15));
  lv_obj_set_y(ui_Screen1Zone2, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone2, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone2, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus2 = lv_label_create(ui_Screen1Zone2);
  lv_obj_set_width(ui_Screen1ZoneStatus2, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus2, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus2, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus2, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus2, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus2, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar2 = lv_label_create(ui_Screen1Zone2);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar2, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar2, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar2, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar2, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum2 = lv_label_create(ui_Screen1Zone2);
  lv_obj_set_width(ui_Screen1ZoneNum2, 55);
  lv_obj_set_height(ui_Screen1ZoneNum2, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum2, 0);
  lv_obj_set_y(ui_Screen1ZoneNum2, 13);
  lv_obj_set_align(ui_Screen1ZoneNum2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum2, "2");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum2, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum2, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone3 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone3, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone3, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone3, lv_pct(33));
  lv_obj_set_y(ui_Screen1Zone3, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone3, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone3, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus3 = lv_label_create(ui_Screen1Zone3);
  lv_obj_set_width(ui_Screen1ZoneStatus3, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus3, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus3, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus3, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus3, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus3, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar3 = lv_label_create(ui_Screen1Zone3);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar3, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar3, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar3, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar3, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum3 = lv_label_create(ui_Screen1Zone3);
  lv_obj_set_width(ui_Screen1ZoneNum3, 55);
  lv_obj_set_height(ui_Screen1ZoneNum3, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum3, 0);
  lv_obj_set_y(ui_Screen1ZoneNum3, 13);
  lv_obj_set_align(ui_Screen1ZoneNum3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum3, "3");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum3, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum3, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum3, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone4 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone4, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone4, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone4, lv_pct(51));
  lv_obj_set_y(ui_Screen1Zone4, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone4, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone4, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus4 = lv_label_create(ui_Screen1Zone4);
  lv_obj_set_width(ui_Screen1ZoneStatus4, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus4, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus4, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus4, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus4, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus4, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar4 = lv_label_create(ui_Screen1Zone4);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar4, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar4, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar4, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar4, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum4 = lv_label_create(ui_Screen1Zone4);
  lv_obj_set_width(ui_Screen1ZoneNum4, 55);
  lv_obj_set_height(ui_Screen1ZoneNum4, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum4, 0);
  lv_obj_set_y(ui_Screen1ZoneNum4, 13);
  lv_obj_set_align(ui_Screen1ZoneNum4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum4, "4");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum4, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum4, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum4, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone5 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone5, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone5, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone5, lv_pct(69));
  lv_obj_set_y(ui_Screen1Zone5, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone5, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone5, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus5 = lv_label_create(ui_Screen1Zone5);
  lv_obj_set_width(ui_Screen1ZoneStatus5, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus5, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus5, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus5, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus5, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus5, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar5 = lv_label_create(ui_Screen1Zone5);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar5, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar5, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar5, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar5, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar5, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum5 = lv_label_create(ui_Screen1Zone5);
  lv_obj_set_width(ui_Screen1ZoneNum5, 55);
  lv_obj_set_height(ui_Screen1ZoneNum5, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum5, 0);
  lv_obj_set_y(ui_Screen1ZoneNum5, 13);
  lv_obj_set_align(ui_Screen1ZoneNum5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum5, "5");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum5, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum5, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum5, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1Zone6 = lv_obj_create(ui_Screen1StatusPanel);
  lv_obj_set_width(ui_Screen1Zone6, lv_pct(15));
  lv_obj_set_height(ui_Screen1Zone6, lv_pct(120));
  lv_obj_set_x(ui_Screen1Zone6, lv_pct(87));
  lv_obj_set_y(ui_Screen1Zone6, lv_pct(-10));
  lv_obj_clear_flag(ui_Screen1Zone6, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1Zone6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1Zone6, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1Zone6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneStatus6 = lv_label_create(ui_Screen1Zone6);
  lv_obj_set_width(ui_Screen1ZoneStatus6, 55);
  lv_obj_set_height(ui_Screen1ZoneStatus6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneStatus6, 0);
  lv_obj_set_y(ui_Screen1ZoneStatus6, -10);
  lv_obj_set_align(ui_Screen1ZoneStatus6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneStatus6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneStatus6, "start");
  lv_obj_set_style_text_align(ui_Screen1ZoneStatus6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneStatus6, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneStatus6, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneStatus6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneStatus6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneStatus6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneFlowmetar6 = lv_label_create(ui_Screen1Zone6);
  lv_obj_set_width(ui_Screen1ZoneFlowmetar6, 55);
  lv_obj_set_height(ui_Screen1ZoneFlowmetar6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1ZoneFlowmetar6, 0);
  lv_obj_set_y(ui_Screen1ZoneFlowmetar6, 55);
  lv_obj_set_align(ui_Screen1ZoneFlowmetar6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneFlowmetar6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneFlowmetar6, "600");
  lv_obj_set_style_text_align(ui_Screen1ZoneFlowmetar6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneFlowmetar6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneFlowmetar6, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneFlowmetar6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneFlowmetar6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneFlowmetar6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneFlowmetar6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneFlowmetar6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1ZoneNum6 = lv_label_create(ui_Screen1Zone6);
  lv_obj_set_width(ui_Screen1ZoneNum6, 55);
  lv_obj_set_height(ui_Screen1ZoneNum6, lv_pct(60));
  lv_obj_set_x(ui_Screen1ZoneNum6, 0);
  lv_obj_set_y(ui_Screen1ZoneNum6, 13);
  lv_obj_set_align(ui_Screen1ZoneNum6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_Screen1ZoneNum6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1ZoneNum6, "6");
  lv_obj_set_style_text_align(ui_Screen1ZoneNum6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_Screen1ZoneNum6, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_Screen1ZoneNum6, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1ZoneNum6, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1ZoneNum6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1ZoneNum6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_Screen1ZoneNum6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_Screen1ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ProductSetButton = lv_btn_create(ui_Screen1);
  lv_obj_set_width(ui_ProductSetButton, 112);
  lv_obj_set_height(ui_ProductSetButton, 40);
  lv_obj_set_x(ui_ProductSetButton, 169);
  lv_obj_set_y(ui_ProductSetButton, 60);
  lv_obj_set_align(ui_ProductSetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_ProductSetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_ProductSetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_ProductSetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ProductSetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ProductSetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ProductSetButtonLabel = lv_label_create(ui_ProductSetButton);
  lv_obj_set_width(ui_ProductSetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_ProductSetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_ProductSetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_ProductSetButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ProductSetButtonLabel, "Product Set");
  lv_obj_set_style_text_color(ui_ProductSetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_ProductSetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_OperationSetButton = lv_btn_create(ui_Screen1);
  lv_obj_set_width(ui_OperationSetButton, 112);
  lv_obj_set_height(ui_OperationSetButton, 40);
  lv_obj_set_x(ui_OperationSetButton, 170);
  lv_obj_set_y(ui_OperationSetButton, 120);
  lv_obj_set_align(ui_OperationSetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_OperationSetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_OperationSetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_OperationSetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_OperationSetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_OperationSetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_OperationSetButtonLabel = lv_label_create(ui_OperationSetButton);
  lv_obj_set_width(ui_OperationSetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_OperationSetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_OperationSetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_OperationSetButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_OperationSetButtonLabel, "Operation Set");
  lv_obj_set_style_text_color(ui_OperationSetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_OperationSetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_FlowResetButton = lv_btn_create(ui_Screen1);
  lv_obj_set_width(ui_FlowResetButton, 112);
  lv_obj_set_height(ui_FlowResetButton, 40);
  lv_obj_set_x(ui_FlowResetButton, -171);
  lv_obj_set_y(ui_FlowResetButton, 60);
  lv_obj_set_align(ui_FlowResetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_FlowResetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_FlowResetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_FlowResetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_FlowResetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_FlowResetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_FlowResetButtonLabel = lv_label_create(ui_FlowResetButton);
  lv_obj_set_width(ui_FlowResetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_FlowResetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_FlowResetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_FlowResetButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_FlowResetButtonLabel, "Flow Reset");
  lv_obj_set_style_text_color(ui_FlowResetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_FlowResetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingResetButton = lv_btn_create(ui_Screen1);
  lv_obj_set_width(ui_SettingResetButton, 112);
  lv_obj_set_height(ui_SettingResetButton, 40);
  lv_obj_set_x(ui_SettingResetButton, -171);
  lv_obj_set_y(ui_SettingResetButton, 120);
  lv_obj_set_align(ui_SettingResetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SettingResetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SettingResetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_SettingResetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SettingResetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SettingResetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingResetButtonLabel = lv_label_create(ui_SettingResetButton);
  lv_obj_set_width(ui_SettingResetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingResetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_SettingResetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_SettingResetButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_SettingResetButtonLabel, "Setting Reset");
  lv_obj_set_style_text_color(ui_SettingResetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SettingResetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1FICLabel = lv_label_create(ui_Screen1);
  lv_obj_set_width(ui_Screen1FICLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1FICLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1FICLabel, -178);
  lv_obj_set_y(ui_Screen1FICLabel, -139);
  lv_obj_set_align(ui_Screen1FICLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Screen1FICLabel, "GreenLABS FIC");
  lv_obj_set_style_text_color(ui_Screen1FICLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Screen1FICLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1TimeLabel = lv_label_create(ui_Screen1);
  lv_obj_set_width(ui_Screen1TimeLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1TimeLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1TimeLabel, 166);
  lv_obj_set_y(ui_Screen1TimeLabel, -139);
  lv_obj_set_align(ui_Screen1TimeLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Screen1TimeLabel, "2022-12-20 11:20:20");
  lv_obj_set_style_text_color(ui_Screen1TimeLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Screen1TimeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1OperationListPanel = lv_obj_create(ui_Screen1);
  lv_obj_set_width(ui_Screen1OperationListPanel, 198);
  lv_obj_set_height(ui_Screen1OperationListPanel, 119);
  lv_obj_set_x(ui_Screen1OperationListPanel, -3);
  lv_obj_set_y(ui_Screen1OperationListPanel, 83);
  lv_obj_set_align(ui_Screen1OperationListPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_Screen1OperationListPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen1OperationListPanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1OperationListLabel1 = lv_label_create(ui_Screen1OperationListPanel);
  lv_obj_set_width(ui_Screen1OperationListLabel1, lv_pct(116));
  lv_obj_set_height(ui_Screen1OperationListLabel1, lv_pct(25));
  lv_obj_set_x(ui_Screen1OperationListLabel1, 0);
  lv_obj_set_y(ui_Screen1OperationListLabel1, -40);
  lv_obj_set_align(ui_Screen1OperationListLabel1, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen1OperationListLabel1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1OperationListLabel1, "1.");
  lv_obj_set_style_text_font(ui_Screen1OperationListLabel1, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_Screen1OperationListLabel1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_Screen1OperationListLabel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1OperationListLabel1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1OperationListLabel1, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1OperationListLabel2 = lv_label_create(ui_Screen1OperationListPanel);
  lv_obj_set_width(ui_Screen1OperationListLabel2, lv_pct(116));
  lv_obj_set_height(ui_Screen1OperationListLabel2, lv_pct(25));
  lv_obj_set_x(ui_Screen1OperationListLabel2, 0);
  lv_obj_set_y(ui_Screen1OperationListLabel2, -15);
  lv_obj_set_align(ui_Screen1OperationListLabel2, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen1OperationListLabel2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1OperationListLabel2, "2.");
  lv_obj_set_style_text_font(ui_Screen1OperationListLabel2, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_Screen1OperationListLabel2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_Screen1OperationListLabel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1OperationListLabel2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1OperationListLabel2, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1OperationListLabel3 = lv_label_create(ui_Screen1OperationListPanel);
  lv_obj_set_width(ui_Screen1OperationListLabel3, lv_pct(116));
  lv_obj_set_height(ui_Screen1OperationListLabel3, lv_pct(25));
  lv_obj_set_x(ui_Screen1OperationListLabel3, 0);
  lv_obj_set_y(ui_Screen1OperationListLabel3, 10);
  lv_obj_set_align(ui_Screen1OperationListLabel3, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen1OperationListLabel3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1OperationListLabel3, "3.");
  lv_obj_set_style_text_font(ui_Screen1OperationListLabel3, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_Screen1OperationListLabel3, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_Screen1OperationListLabel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1OperationListLabel3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1OperationListLabel3, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1OperationListLabel14 = lv_label_create(ui_Screen1OperationListPanel);
  lv_obj_set_width(ui_Screen1OperationListLabel14, lv_pct(116));
  lv_obj_set_height(ui_Screen1OperationListLabel14, lv_pct(25));
  lv_obj_set_x(ui_Screen1OperationListLabel14, 0);
  lv_obj_set_y(ui_Screen1OperationListLabel14, 35);
  lv_obj_set_align(ui_Screen1OperationListLabel14, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen1OperationListLabel14, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1OperationListLabel14, "4.");
  lv_obj_set_style_text_font(ui_Screen1OperationListLabel14, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_Screen1OperationListLabel14, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_Screen1OperationListLabel14, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1OperationListLabel14, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1OperationListLabel14, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1MasterStatusLabel = lv_label_create(ui_Screen1);
  lv_obj_set_width(ui_Screen1MasterStatusLabel, lv_pct(12));
  lv_obj_set_height(ui_Screen1MasterStatusLabel, lv_pct(6));
  lv_obj_set_x(ui_Screen1MasterStatusLabel, -198);
  lv_obj_set_y(ui_Screen1MasterStatusLabel, 21);
  lv_obj_set_align(ui_Screen1MasterStatusLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen1MasterStatusLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen1MasterStatusLabel, "master");
  lv_obj_set_style_radius(ui_Screen1MasterStatusLabel, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen1MasterStatusLabel, lv_color_hex(0xF10C53), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen1MasterStatusLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_Screen1MasterStatusLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_Screen1MasterStatusLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_Screen1MasterStatusLabel, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_Screen1MasterStatusLabel, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_OperationSetButton, ui_event_OperationSetButton, LV_EVENT_ALL, NULL);
}
void ui_Screen2_screen_init(void) {
  ui_Screen2 = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Screen2, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_Screen2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_Screen2, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen2MainPanel = lv_obj_create(ui_Screen2);
  lv_obj_set_width(ui_Screen2MainPanel, 445);
  lv_obj_set_height(ui_Screen2MainPanel, 292);
  lv_obj_set_x(ui_Screen2MainPanel, -2);
  lv_obj_set_y(ui_Screen2MainPanel, -1);
  lv_obj_set_align(ui_Screen2MainPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_Screen2MainPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_Screen2_Keyboard = lv_keyboard_create(ui_Screen2MainPanel);
  lv_obj_set_width(ui_Screen2_Keyboard, 410);
  lv_obj_set_height(ui_Screen2_Keyboard, 180);
  lv_obj_set_x(ui_Screen2_Keyboard, 2);
  lv_obj_set_y(ui_Screen2_Keyboard, 47);
  lv_obj_set_align(ui_Screen2_Keyboard, LV_ALIGN_CENTER);

  ui_Screen2TextArea = lv_textarea_create(ui_Screen2);
  lv_obj_set_width(ui_Screen2TextArea, 60);
  lv_obj_set_height(ui_Screen2TextArea, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_Screen2TextArea, -162);
  lv_obj_set_y(ui_Screen2TextArea, -90);
  lv_obj_set_align(ui_Screen2TextArea, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_Screen2TextArea, "Placeholder...");
  lv_textarea_set_one_line(ui_Screen2TextArea, true);

  ui_Screen2TextArea1 = lv_textarea_create(ui_Screen2);
  lv_obj_set_width(ui_Screen2TextArea1, 70);
  lv_obj_set_height(ui_Screen2TextArea1, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_Screen2TextArea1, -91);
  lv_obj_set_y(ui_Screen2TextArea1, -90);
  lv_obj_set_align(ui_Screen2TextArea1, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_Screen2TextArea1, "Placeholder...");
  lv_textarea_set_one_line(ui_Screen2TextArea1, true);

  ui_Screen2TextArea2 = lv_textarea_create(ui_Screen2);
  lv_obj_set_width(ui_Screen2TextArea2, 70);
  lv_obj_set_height(ui_Screen2TextArea2, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_Screen2TextArea2, -13);
  lv_obj_set_y(ui_Screen2TextArea2, -91);
  lv_obj_set_align(ui_Screen2TextArea2, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_Screen2TextArea2, "Placeholder...");
  lv_textarea_set_one_line(ui_Screen2TextArea2, true);

  ui_Screen2SaveButton = lv_btn_create(ui_Screen2);
  lv_obj_set_width(ui_Screen2SaveButton, 70);
  lv_obj_set_height(ui_Screen2SaveButton, 37);
  lv_obj_set_x(ui_Screen2SaveButton, 123);
  lv_obj_set_y(ui_Screen2SaveButton, -94);
  lv_obj_set_align(ui_Screen2SaveButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Screen2SaveButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Screen2SaveButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Screen2SaveButton, lv_color_hex(0x4E4D4D), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_Screen2SaveButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen2SaveButtonLabel = lv_label_create(ui_Screen2SaveButton);
  lv_obj_set_width(ui_Screen2SaveButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen2SaveButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_Screen2SaveButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_long_mode(ui_Screen2SaveButtonLabel, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_Screen2SaveButtonLabel, "Save");

  ui_Scree2ExampleLaBel1 = lv_label_create(ui_Screen2);
  lv_obj_set_width(ui_Scree2ExampleLaBel1, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Scree2ExampleLaBel1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Scree2ExampleLaBel1, -89);
  lv_obj_set_y(ui_Scree2ExampleLaBel1, -123);
  lv_obj_set_align(ui_Scree2ExampleLaBel1, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Scree2ExampleLaBel1, "flow rate / area / time set");

  lv_keyboard_set_textarea(ui_Screen2_Keyboard, ui_Screen2TextArea);
  lv_obj_add_event_cb(ui_Screen2TextArea, ui_event_Screen2TextArea, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_Screen2TextArea1, ui_event_Screen2TextArea1, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_Screen2TextArea2, ui_event_Screen2TextArea2, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_Screen2SaveButton, ui_event_Screen2SaveButton, LV_EVENT_ALL, NULL);
}

void ui_init(void) {
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                            false, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);
  ui_Screen1_screen_init();
  ui_Screen2_screen_init();
  lv_disp_load_scr(ui_Screen1);
}
