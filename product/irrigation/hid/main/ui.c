// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#include <time.h>
#include <stdio.h>

#include "gui.h"
#include "ui_helpers.h"
#include "hid_config.h"
#include "comm_packet.h"

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_Main;
lv_obj_t *ui_MainStatusPanel;
lv_obj_t *ui_ZonePanel1;
lv_obj_t *ui_ZoneStatus1;
lv_obj_t *ui_ZoneFlowmeter1;
lv_obj_t *ui_ZoneNum1;
lv_obj_t *ui_ZonePanel2;
lv_obj_t *ui_ZoneStatus2;
lv_obj_t *ui_ZoneFlowmeter2;
lv_obj_t *ui_ZoneNum2;
lv_obj_t *ui_ZonePanel3;
lv_obj_t *ui_ZoneStatus3;
lv_obj_t *ui_ZoneFlowmeter3;
lv_obj_t *ui_ZoneNum3;
lv_obj_t *ui_ZonePanel4;
lv_obj_t *ui_ZoneStatus4;
lv_obj_t *ui_ZoneFlowmeter4;
lv_obj_t *ui_ZoneNum4;
lv_obj_t *ui_ZonePanel5;
lv_obj_t *ui_ZoneStatus5;
lv_obj_t *ui_ZoneFlowmeter5;
lv_obj_t *ui_ZoneNum5;
lv_obj_t *ui_ZonePanel6;
lv_obj_t *ui_ZoneStatus6;
lv_obj_t *ui_ZoneFlowmeter6;
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
lv_obj_t *ui_OperationList;
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
lv_obj_t *ui_Zone1;
lv_obj_t *ui_Zone2;
lv_obj_t *ui_Zone3;
lv_obj_t *ui_Zone4;
lv_obj_t *ui_Zone5;
lv_obj_t *ui_Zone6;
lv_obj_t *ui_ZoneAreaLabel;
lv_obj_t *ui_Screen1FICLabel;
lv_obj_t *ui_Screen1TimeLabel;
lv_obj_t *ui_Screen1DateLabel;

lv_obj_t *ui_SettingSelect_screen;
lv_obj_t *ui_SS_MainPanel;
void ui_event_SS_OpSet_Button(lv_event_t *e);
lv_obj_t *ui_SS_OpSet_Button;
lv_obj_t *ui_SS_OpSet_Label;
void ui_event_SS_Device_mag_Button(lv_event_t *e);
lv_obj_t *ui_SS_Device_mag_Button;
lv_obj_t *ui_SS_Device_mag_Label;
lv_obj_t *ui_DeviceManager_screen;
lv_obj_t *ui_DM_MainPanel;
lv_obj_t *ui_DM_Roller;
lv_obj_t *ui_DM_Add_Button;
lv_obj_t *ui_DM_Add_Label;
void ui_event_DM_Del_Button(lv_event_t *e);
lv_obj_t *ui_DM_Del_Button;
lv_obj_t *ui_DM_Del_Label;
void ui_event_DM_Exit_Button(lv_event_t *e);
lv_obj_t *ui_DM_Exit_Button;
lv_obj_t *ui_DM_Exit_Label;
lv_obj_t *ui_DeviceManagerReg_screen;
lv_obj_t *ui_DMR_MainPanel;
lv_obj_t *ui_DMR_Keyboard;
lv_obj_t *ui_DMR_TextArea;
lv_obj_t *ui_DMR_Reg_Button;
lv_obj_t *ui_DMR_Reg_Label;
void ui_event_DMR_Exit_Button(lv_event_t *e);
lv_obj_t *ui_DMR_Exit_Button;
lv_obj_t *ui_DMR_Exit_Label;

static lv_style_t style_clock;
char timeString[9];
char dateString[30];

const char *DAY[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const char *MONTH[] = { "January", "February", "March",     "April",   "May",      "June",
                        "July",    "August",   "September", "October", "November", "December" };

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif

static void status_change_cb(void *s, lv_msg_t *m) {
  LV_UNUSED(s);

  lvgl_acquire();

  unsigned int msg_id = lv_msg_get_id(m);

  switch (msg_id) {
    case MSG_TIME_SYNCED: enable_buttons(); break;
    case MSG_BATTERY_STATUS: break;
    case MSG_RESPONSE_STATUS: {
      irrigation_message_t *msg_payload = (irrigation_message_t *)lv_msg_get_payload(m);
      if (msg_payload->receive_type == SET_CONFIG) {
        disable_start_button();
      } else if (msg_payload->receive_type == FORCE_STOP) {
        char op_msg[128] = { 0 };
        int zone_id = msg_payload->deviceId;
        int flow_value = msg_payload->flow_value;
        if (zone_id >= 1 && zone_id <= 6) {
          set_zone_status(zone_id, false);
          set_zone_number(zone_id, false);
          set_zone_flow_value(zone_id, flow_value);
          enable_start_button();
          snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
          add_operation_list(op_msg);
        }
      }
    } break;
    case MSG_IRRIGATION_STATUS: {
      char op_msg[128] = { 0 };
      irrigation_message_t *msg_payload = (irrigation_message_t *)lv_msg_get_payload(m);
      int zone_id = msg_payload->deviceId;
      if (msg_payload->sender_type == START_FLOW) {
        set_zone_status(zone_id, true);
        set_zone_number(zone_id, true);
        disable_start_button();
        snprintf(op_msg, sizeof(op_msg), "Start to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
        add_operation_list(op_msg);
      } else if (msg_payload->sender_type == ZONE_COMPLETE) {
        int flow_value = msg_payload->flow_value;
        set_zone_status(zone_id, false);
        set_zone_number(zone_id, false);
        snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
        add_operation_list(op_msg);
      } else if (msg_payload->sender_type == ALL_COMPLETE) {
        enable_start_button();
      }
    } break;
    default: break;
  }

  lvgl_release();
}

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
    OnSettingEvent(e);
    _ui_screen_change(ui_SettingSelect_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 0, 0);
    //_ui_screen_change(ui_Setting, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0);
  }
}
void ui_event_FlowRateText(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    OnFlowRateEvent(e);
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
    _ui_screen_change(ui_Main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 0, 0);
  }
}
void ui_event_SettingCancelButton(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 0, 0);
  }
}

void ui_event_SS_OpSet_Button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Setting, LV_SCR_LOAD_ANIM_MOVE_LEFT, 0, 0);
  }
}

void ui_event_SS_Device_mag_Button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_DeviceManager_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 0, 0);
  }
}
void ui_event_DM_Del_Button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_Main, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 0, 0);
  }
}
void ui_event_DM_Exit_Button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_DeviceManagerReg_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 0, 0);
  }
}
void ui_event_DMR_Exit_Button(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(ui_DeviceManager_screen, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 0, 0);
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
  // lv_obj_set_y(ui_MainStatusPanel, -87);
  lv_obj_set_y(ui_MainStatusPanel, -60);
  lv_obj_set_align(ui_MainStatusPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_MainStatusPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_MainStatusPanel, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel1 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel1, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel1, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel1, lv_pct(-3));
  lv_obj_set_y(ui_ZonePanel1, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel1, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel1, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus1 = lv_label_create(ui_ZonePanel1);
  lv_obj_set_width(ui_ZoneStatus1, 55);
  lv_obj_set_height(ui_ZoneStatus1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus1, 0);
  lv_obj_set_y(ui_ZoneStatus1, -10);
  lv_obj_set_align(ui_ZoneStatus1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus1, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus1, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus1, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus1, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter1 = lv_label_create(ui_ZonePanel1);
  lv_obj_set_width(ui_ZoneFlowmeter1, 55);
  lv_obj_set_height(ui_ZoneFlowmeter1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter1, 0);
  lv_obj_set_y(ui_ZoneFlowmeter1, 55);
  lv_obj_set_align(ui_ZoneFlowmeter1, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter1, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter1, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter1, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter1, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum1 = lv_label_create(ui_ZonePanel1);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum1, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum1, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum1, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum1, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel2 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel2, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel2, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel2, lv_pct(15));
  lv_obj_set_y(ui_ZonePanel2, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel2, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel2, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus2 = lv_label_create(ui_ZonePanel2);
  lv_obj_set_width(ui_ZoneStatus2, 55);
  lv_obj_set_height(ui_ZoneStatus2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus2, 0);
  lv_obj_set_y(ui_ZoneStatus2, -10);
  lv_obj_set_align(ui_ZoneStatus2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus2, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus2, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus2, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus2, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter2 = lv_label_create(ui_ZonePanel2);
  lv_obj_set_width(ui_ZoneFlowmeter2, 55);
  lv_obj_set_height(ui_ZoneFlowmeter2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter2, 0);
  lv_obj_set_y(ui_ZoneFlowmeter2, 55);
  lv_obj_set_align(ui_ZoneFlowmeter2, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter2, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter2, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter2, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum2 = lv_label_create(ui_ZonePanel2);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum2, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum2, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum2, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum2, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel3 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel3, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel3, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel3, lv_pct(33));
  lv_obj_set_y(ui_ZonePanel3, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel3, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel3, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus3 = lv_label_create(ui_ZonePanel3);
  lv_obj_set_width(ui_ZoneStatus3, 55);
  lv_obj_set_height(ui_ZoneStatus3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus3, 0);
  lv_obj_set_y(ui_ZoneStatus3, -10);
  lv_obj_set_align(ui_ZoneStatus3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus3, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus3, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus3, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus3, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter3 = lv_label_create(ui_ZonePanel3);
  lv_obj_set_width(ui_ZoneFlowmeter3, 55);
  lv_obj_set_height(ui_ZoneFlowmeter3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter3, 0);
  lv_obj_set_y(ui_ZoneFlowmeter3, 55);
  lv_obj_set_align(ui_ZoneFlowmeter3, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter3, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter3, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter3, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter3, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum3 = lv_label_create(ui_ZonePanel3);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum3, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum3, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum3, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum3, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel4 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel4, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel4, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel4, lv_pct(51));
  lv_obj_set_y(ui_ZonePanel4, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel4, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel4, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus4 = lv_label_create(ui_ZonePanel4);
  lv_obj_set_width(ui_ZoneStatus4, 55);
  lv_obj_set_height(ui_ZoneStatus4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus4, 0);
  lv_obj_set_y(ui_ZoneStatus4, -10);
  lv_obj_set_align(ui_ZoneStatus4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus4, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus4, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus4, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus4, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter4 = lv_label_create(ui_ZonePanel4);
  lv_obj_set_width(ui_ZoneFlowmeter4, 55);
  lv_obj_set_height(ui_ZoneFlowmeter4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter4, 0);
  lv_obj_set_y(ui_ZoneFlowmeter4, 55);
  lv_obj_set_align(ui_ZoneFlowmeter4, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter4, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter4, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter4, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter4, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter4, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum4 = lv_label_create(ui_ZonePanel4);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum4, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum4, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum4, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum4, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel5 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel5, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel5, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel5, lv_pct(69));
  lv_obj_set_y(ui_ZonePanel5, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel5, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel5, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus5 = lv_label_create(ui_ZonePanel5);
  lv_obj_set_width(ui_ZoneStatus5, 55);
  lv_obj_set_height(ui_ZoneStatus5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus5, 0);
  lv_obj_set_y(ui_ZoneStatus5, -10);
  lv_obj_set_align(ui_ZoneStatus5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus5, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus5, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus5, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus5, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter5 = lv_label_create(ui_ZonePanel5);
  lv_obj_set_width(ui_ZoneFlowmeter5, 55);
  lv_obj_set_height(ui_ZoneFlowmeter5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter5, 0);
  lv_obj_set_y(ui_ZoneFlowmeter5, 55);
  lv_obj_set_align(ui_ZoneFlowmeter5, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter5, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter5, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter5, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter5, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter5, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum5 = lv_label_create(ui_ZonePanel5);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum5, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum5, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum5, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum5, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum5, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum5, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZonePanel6 = lv_obj_create(ui_MainStatusPanel);
  lv_obj_set_width(ui_ZonePanel6, lv_pct(15));
  lv_obj_set_height(ui_ZonePanel6, lv_pct(120));
  lv_obj_set_x(ui_ZonePanel6, lv_pct(87));
  lv_obj_set_y(ui_ZonePanel6, lv_pct(-10));
  lv_obj_clear_flag(ui_ZonePanel6, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_ZonePanel6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZonePanel6, lv_color_hex(0xABABA2), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZonePanel6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneStatus6 = lv_label_create(ui_ZonePanel6);
  lv_obj_set_width(ui_ZoneStatus6, 55);
  lv_obj_set_height(ui_ZoneStatus6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneStatus6, 0);
  lv_obj_set_y(ui_ZoneStatus6, -10);
  lv_obj_set_align(ui_ZoneStatus6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneStatus6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneStatus6, "stop");
  lv_obj_set_style_text_align(ui_ZoneStatus6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneStatus6, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_set_style_bg_color(ui_ZoneStatus6, lv_color_hex(0x1BFD32), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneStatus6, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneStatus6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneStatus6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneStatus6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneStatus6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneFlowmeter6 = lv_label_create(ui_ZonePanel6);
  lv_obj_set_width(ui_ZoneFlowmeter6, 55);
  lv_obj_set_height(ui_ZoneFlowmeter6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneFlowmeter6, 0);
  lv_obj_set_y(ui_ZoneFlowmeter6, 55);
  lv_obj_set_align(ui_ZoneFlowmeter6, LV_ALIGN_TOP_MID);
  lv_label_set_long_mode(ui_ZoneFlowmeter6, LV_LABEL_LONG_DOT);
  lv_label_set_text(ui_ZoneFlowmeter6, "0");
  lv_obj_set_style_text_align(ui_ZoneFlowmeter6, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_radius(ui_ZoneFlowmeter6, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneFlowmeter6, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneFlowmeter6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneFlowmeter6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneFlowmeter6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneFlowmeter6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneFlowmeter6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_ZoneNum6 = lv_label_create(ui_ZonePanel6);
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
  // lv_obj_set_style_bg_color(ui_ZoneNum6, lv_color_hex(0xFAFAF8), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ZoneNum6, lv_color_hex(0xFF1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_ZoneNum6, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_side(ui_ZoneNum6, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_color(ui_ZoneNum6, lv_color_hex(0xC3CE1A), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_outline_opa(ui_ZoneNum6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_StartButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_StartButton, 100);
  lv_obj_set_height(ui_StartButton, 40);
  lv_obj_set_x(ui_StartButton, -175);
  lv_obj_set_y(ui_StartButton, 31);
  lv_obj_set_align(ui_StartButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_StartButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_StartButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_StartButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_StartButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_StartButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_add_state(ui_StartButton, LV_STATE_DISABLED);
  //_ui_state_modify(ui_StartButton, _UI_MODIFY_STATE_ADD, LV_STATE_DISABLED);

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
  lv_obj_set_width(ui_StopButton, 100);
  lv_obj_set_height(ui_StopButton, 40);
  lv_obj_set_x(ui_StopButton, -58);
  lv_obj_set_y(ui_StopButton, 31);
  lv_obj_set_align(ui_StopButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_StopButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_StopButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_StopButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_StopButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_StopButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_add_state(ui_StopButton, LV_STATE_DISABLED);
  // _ui_state_modify(ui_StopButton, _UI_MODIFY_STATE_ADD, LV_STATE_DISABLED);

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
  lv_obj_set_width(ui_ResetButton, 100);
  lv_obj_set_height(ui_ResetButton, 40);
  lv_obj_set_x(ui_ResetButton, 175);
  lv_obj_set_y(ui_ResetButton, 31);
  lv_obj_set_align(ui_ResetButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_ResetButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_ResetButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_ResetButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_ResetButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_ResetButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_add_state(ui_ResetButton, LV_STATE_DISABLED);
  // _ui_state_modify(ui_ResetButton, _UI_MODIFY_STATE_ADD, LV_STATE_DISABLED);

  ui_ResetButtonLabel = lv_label_create(ui_ResetButton);
  lv_obj_set_width(ui_ResetButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_ResetButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_ResetButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_ResetButtonLabel, "Reset");
  lv_obj_set_style_text_color(ui_ResetButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_ResetButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_ResetButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_SettingButton, 100);
  lv_obj_set_height(ui_SettingButton, 40);
  lv_obj_set_x(ui_SettingButton, 59);
  lv_obj_set_y(ui_SettingButton, 31);
  lv_obj_set_align(ui_SettingButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SettingButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SettingButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_SettingButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SettingButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SettingButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_add_state(ui_SettingButton, LV_STATE_DISABLED);
  // _ui_state_modify(ui_SettingButton, _UI_MODIFY_STATE_ADD, LV_STATE_DISABLED);

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
  lv_obj_set_width(ui_OperationListPanel, 450);
  lv_obj_set_height(ui_OperationListPanel, 89);
  lv_obj_set_x(ui_OperationListPanel, 1);
  lv_obj_set_y(ui_OperationListPanel, 105);
  lv_obj_set_align(ui_OperationListPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_OperationListPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_OperationListPanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_OperationList = lv_textarea_create(ui_OperationListPanel);
  lv_obj_set_width(ui_OperationList, 428);
  lv_obj_set_height(ui_OperationList, 70);
  lv_obj_set_x(ui_OperationList, 1);
  lv_obj_set_y(ui_OperationList, 0);
  lv_obj_set_align(ui_OperationList, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_OperationList, "Placeholder...");

  lv_obj_add_event_cb(ui_StartButton, ui_event_StartButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_StopButton, ui_event_StopButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_ResetButton, ui_event_ResetButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingButton, ui_event_SettingButton, LV_EVENT_ALL, NULL);

  ui_Screen1FICLabel = lv_label_create(ui_Main);
  lv_obj_set_width(ui_Screen1FICLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1FICLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1FICLabel, -178);
  lv_obj_set_y(ui_Screen1FICLabel, -139);
  lv_obj_set_align(ui_Screen1FICLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Screen1FICLabel, "GreenLABS FIC");
  lv_obj_set_style_text_color(ui_Screen1FICLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Screen1FICLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1TimeLabel = lv_label_create(ui_Main);
  lv_obj_set_width(ui_Screen1TimeLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1TimeLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1TimeLabel, 175);
  lv_obj_set_y(ui_Screen1TimeLabel, -139);
  lv_obj_set_align(ui_Screen1TimeLabel, LV_ALIGN_CENTER);
  lv_obj_add_style(ui_Screen1TimeLabel, &style_clock, 0);
  lv_label_set_text(ui_Screen1TimeLabel, timeString);
  lv_label_set_long_mode(ui_Screen1TimeLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(ui_Screen1TimeLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Screen1TimeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_Screen1DateLabel = lv_label_create(ui_Main);
  lv_obj_set_width(ui_Screen1DateLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1DateLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1DateLabel, 100);
  lv_obj_set_y(ui_Screen1DateLabel, -139);
  lv_obj_set_align(ui_Screen1DateLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Screen1DateLabel, dateString);
  lv_obj_set_style_text_color(ui_Screen1DateLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_Screen1DateLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
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
  lv_obj_set_x(ui_SettingPanel, 1);
  lv_obj_set_y(ui_SettingPanel, -2);
  lv_obj_set_align(ui_SettingPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_SettingPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                                         LV_OBJ_FLAG_SCROLL_CHAIN);  /// Flags

  ui_SettingKeyboard = lv_keyboard_create(ui_SettingPanel);
  lv_keyboard_set_mode(ui_SettingKeyboard, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_set_width(ui_SettingKeyboard, 410);
  lv_obj_set_height(ui_SettingKeyboard, 163);
  lv_obj_set_x(ui_SettingKeyboard, 2);
  lv_obj_set_y(ui_SettingKeyboard, 54);
  lv_obj_set_align(ui_SettingKeyboard, LV_ALIGN_CENTER);

  ui_SettingTitle = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingTitle, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingTitle, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingTitle, -88);
  lv_obj_set_y(ui_SettingTitle, -126);
  lv_obj_set_align(ui_SettingTitle, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingTitle, "flow rate / time set (Hour, Min)");

  ui_FlowRateText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_FlowRateText, 65);
  lv_obj_set_height(ui_FlowRateText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_FlowRateText, -164);
  lv_obj_set_y(ui_FlowRateText, -91);
  lv_obj_set_align(ui_FlowRateText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_FlowRateText, "Placeholder...");
  lv_textarea_set_one_line(ui_FlowRateText, true);

  ui_TimeHourText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeHourText, 58);
  lv_obj_set_height(ui_TimeHourText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_TimeHourText, -84);
  lv_obj_set_y(ui_TimeHourText, -91);
  lv_obj_set_align(ui_TimeHourText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeHourText, "Placeholder...");
  lv_textarea_set_one_line(ui_TimeHourText, true);

  ui_TimeSeperate = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_TimeSeperate, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_TimeSeperate, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_TimeSeperate, -41);
  lv_obj_set_y(ui_TimeSeperate, -91);
  lv_obj_set_align(ui_TimeSeperate, LV_ALIGN_CENTER);
  lv_label_set_text(ui_TimeSeperate, ":");

  ui_TimeMinuteText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeMinuteText, 58);
  lv_obj_set_height(ui_TimeMinuteText, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_TimeMinuteText, 2);
  lv_obj_set_y(ui_TimeMinuteText, -91);
  lv_obj_set_align(ui_TimeMinuteText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeMinuteText, "Placeholder...");
  lv_textarea_set_one_line(ui_TimeMinuteText, true);

  ui_SettingSaveButton = lv_btn_create(ui_Setting);
  lv_obj_set_width(ui_SettingSaveButton, 70);
  lv_obj_set_height(ui_SettingSaveButton, 37);
  lv_obj_set_x(ui_SettingSaveButton, 89);
  lv_obj_set_y(ui_SettingSaveButton, -94);
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
  lv_obj_set_x(ui_SettingCancelButton, 174);
  lv_obj_set_y(ui_SettingCancelButton, -95);
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

  ui_Zone1 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone1, "1");
  lv_obj_set_width(ui_Zone1, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone1, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone1, -126);
  lv_obj_set_y(ui_Zone1, -50);
  lv_obj_set_align(ui_Zone1, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone2 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone2, "2");
  lv_obj_set_width(ui_Zone2, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone2, -73);
  lv_obj_set_y(ui_Zone2, -50);
  lv_obj_set_align(ui_Zone2, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone3 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone3, "3");
  lv_obj_set_width(ui_Zone3, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone3, -20);
  lv_obj_set_y(ui_Zone3, -50);
  lv_obj_set_align(ui_Zone3, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone4 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone4, "4");
  lv_obj_set_width(ui_Zone4, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone4, 33);
  lv_obj_set_y(ui_Zone4, -50);
  lv_obj_set_align(ui_Zone4, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone4, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone5 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone5, "5");
  lv_obj_set_width(ui_Zone5, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone5, 88);
  lv_obj_set_y(ui_Zone5, -50);
  lv_obj_set_align(ui_Zone5, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone5, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone6 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone6, "6");
  lv_obj_set_width(ui_Zone6, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone6, 141);
  lv_obj_set_y(ui_Zone6, -50);
  lv_obj_set_align(ui_Zone6, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone6, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_ZoneAreaLabel = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_ZoneAreaLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_ZoneAreaLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneAreaLabel, -173);
  lv_obj_set_y(ui_ZoneAreaLabel, -49);
  lv_obj_set_align(ui_ZoneAreaLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_ZoneAreaLabel, "Zone : ");

  lv_keyboard_set_textarea(ui_SettingKeyboard, ui_FlowRateText);
  lv_obj_add_event_cb(ui_FlowRateText, ui_event_FlowRateText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_TimeHourText, ui_event_TimeHourText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_TimeMinuteText, ui_event_TimeMinuteText, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingSaveButton, ui_event_SettingSaveButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingCancelButton, ui_event_SettingCancelButton, LV_EVENT_ALL, NULL);
}

void ui_SettingSelect_screen_init(void) {
  ui_SettingSelect_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_SettingSelect_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_SettingSelect_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SettingSelect_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SettingSelect_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_MainPanel = lv_obj_create(ui_SettingSelect_screen);
  lv_obj_set_width(ui_SS_MainPanel, 445);
  lv_obj_set_height(ui_SS_MainPanel, 292);
  lv_obj_set_align(ui_SS_MainPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_SS_MainPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_grad_color(ui_SS_MainPanel, lv_color_hex(0x9FA6F3), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_dir(ui_SS_MainPanel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_OpSet_Button = lv_btn_create(ui_SS_MainPanel);
  lv_obj_set_width(ui_SS_OpSet_Button, 239);
  lv_obj_set_height(ui_SS_OpSet_Button, 50);
  lv_obj_set_x(ui_SS_OpSet_Button, 0);
  lv_obj_set_y(ui_SS_OpSet_Button, -40);
  lv_obj_set_align(ui_SS_OpSet_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SS_OpSet_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SS_OpSet_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_SS_OpSet_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SS_OpSet_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SS_OpSet_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SS_OpSet_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SS_OpSet_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_SS_OpSet_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_SS_OpSet_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_SS_OpSet_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_SS_OpSet_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_OpSet_Label = lv_label_create(ui_SS_OpSet_Button);
  lv_obj_set_width(ui_SS_OpSet_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SS_OpSet_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_SS_OpSet_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SS_OpSet_Label, "OperationSetting");
  lv_obj_set_style_text_color(ui_SS_OpSet_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SS_OpSet_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_Device_mag_Button = lv_btn_create(ui_SS_MainPanel);
  lv_obj_set_width(ui_SS_Device_mag_Button, 239);
  lv_obj_set_height(ui_SS_Device_mag_Button, 50);
  lv_obj_set_x(ui_SS_Device_mag_Button, 0);
  lv_obj_set_y(ui_SS_Device_mag_Button, 40);
  lv_obj_set_align(ui_SS_Device_mag_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_SS_Device_mag_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_SS_Device_mag_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_SS_Device_mag_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SS_Device_mag_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_SS_Device_mag_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_SS_Device_mag_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_SS_Device_mag_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_SS_Device_mag_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_SS_Device_mag_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_SS_Device_mag_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_SS_Device_mag_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_Device_mag_Label = lv_label_create(ui_SS_Device_mag_Button);
  lv_obj_set_width(ui_SS_Device_mag_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SS_Device_mag_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_SS_Device_mag_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SS_Device_mag_Label, "Device Management");
  lv_obj_set_style_text_color(ui_SS_Device_mag_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SS_Device_mag_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_SS_OpSet_Button, ui_event_SS_OpSet_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SS_Device_mag_Button, ui_event_SS_Device_mag_Button, LV_EVENT_ALL, NULL);
}

void ui_DeviceManager_screen_init(void) {
  ui_DeviceManager_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_DeviceManager_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_DeviceManager_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_DeviceManager_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DeviceManager_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_MainPanel = lv_obj_create(ui_DeviceManager_screen);
  lv_obj_set_width(ui_DM_MainPanel, 450);
  lv_obj_set_height(ui_DM_MainPanel, 300);
  lv_obj_set_align(ui_DM_MainPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_DM_MainPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_grad_color(ui_DM_MainPanel, lv_color_hex(0x9FA6F3), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_dir(ui_DM_MainPanel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Roller = lv_roller_create(ui_DM_MainPanel);
  lv_roller_set_options(ui_DM_Roller,
                        "F412FAC088DB\nF412FAC088DB\nF412FAC088DB\nF412FAC088DB\nF412FAC088DB\nF412FAC088DB\nF412FAC0"
                        "88DB\nF412FAC088DB\nF412FAC088DB\n",
                        LV_ROLLER_MODE_NORMAL);
  lv_obj_set_height(ui_DM_Roller, 270);
  lv_obj_set_width(ui_DM_Roller, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_DM_Roller, -100);
  lv_obj_set_y(ui_DM_Roller, 0);
  lv_obj_set_align(ui_DM_Roller, LV_ALIGN_CENTER);

  ui_DM_Add_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Add_Button, 100);
  lv_obj_set_height(ui_DM_Add_Button, 50);
  lv_obj_set_x(ui_DM_Add_Button, 83);
  lv_obj_set_y(ui_DM_Add_Button, 0);
  lv_obj_set_align(ui_DM_Add_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_Add_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DM_Add_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DM_Add_Button, lv_color_hex(0xED2D40), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_Add_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DM_Add_Button, lv_color_hex(0xF0D0D4), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_DM_Add_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_DM_Add_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_DM_Add_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_DM_Add_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_DM_Add_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_DM_Add_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Add_Label = lv_label_create(ui_DM_Add_Button);
  lv_obj_set_width(ui_DM_Add_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_DM_Add_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_DM_Add_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DM_Add_Label, "DELETE");

  ui_DM_Del_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Del_Button, 100);
  lv_obj_set_height(ui_DM_Del_Button, 50);
  lv_obj_set_x(ui_DM_Del_Button, 83);
  lv_obj_set_y(ui_DM_Del_Button, 75);
  lv_obj_set_align(ui_DM_Del_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_Del_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DM_Del_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DM_Del_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_Del_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DM_Del_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_DM_Del_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_DM_Del_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_DM_Del_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_DM_Del_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_DM_Del_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_DM_Del_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Del_Label = lv_label_create(ui_DM_Del_Button);
  lv_obj_set_width(ui_DM_Del_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_DM_Del_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_DM_Del_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DM_Del_Label, "EXIT");
  lv_obj_set_style_text_color(ui_DM_Del_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DM_Del_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Exit_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Exit_Button, 100);
  lv_obj_set_height(ui_DM_Exit_Button, 50);
  lv_obj_set_x(ui_DM_Exit_Button, 83);
  lv_obj_set_y(ui_DM_Exit_Button, -75);
  lv_obj_set_align(ui_DM_Exit_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_Exit_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DM_Exit_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DM_Exit_Button, lv_color_hex(0x11EF39), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DM_Exit_Button, lv_color_hex(0xDBF2DF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_DM_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_DM_Exit_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_DM_Exit_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_DM_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_DM_Exit_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_DM_Exit_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Exit_Label = lv_label_create(ui_DM_Exit_Button);
  lv_obj_set_width(ui_DM_Exit_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_DM_Exit_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_DM_Exit_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DM_Exit_Label, "ADD");

  lv_obj_add_event_cb(ui_DM_Del_Button, ui_event_DM_Del_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DM_Exit_Button, ui_event_DM_Exit_Button, LV_EVENT_ALL, NULL);
}
void ui_DeviceManagerReg_screen_init(void) {
  ui_DeviceManagerReg_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_DeviceManagerReg_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_DeviceManagerReg_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_DeviceManagerReg_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DeviceManagerReg_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_MainPanel = lv_obj_create(ui_DeviceManagerReg_screen);
  lv_obj_set_width(ui_DMR_MainPanel, 445);
  lv_obj_set_height(ui_DMR_MainPanel, 292);
  lv_obj_set_x(ui_DMR_MainPanel, 0);
  lv_obj_set_y(ui_DMR_MainPanel, -1);
  lv_obj_set_align(ui_DMR_MainPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_DMR_MainPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_grad_color(ui_DMR_MainPanel, lv_color_hex(0x9FA6F3), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_dir(ui_DMR_MainPanel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_Keyboard = lv_keyboard_create(ui_DMR_MainPanel);
  lv_obj_set_width(ui_DMR_Keyboard, 440);
  lv_obj_set_height(ui_DMR_Keyboard, 150);
  lv_obj_set_x(ui_DMR_Keyboard, 0);
  lv_obj_set_y(ui_DMR_Keyboard, 70);
  lv_obj_set_align(ui_DMR_Keyboard, LV_ALIGN_CENTER);

  ui_DMR_TextArea = lv_textarea_create(ui_DMR_MainPanel);
  lv_obj_set_width(ui_DMR_TextArea, 415);
  lv_obj_set_height(ui_DMR_TextArea, LV_SIZE_CONTENT);  /// 42
  lv_obj_set_x(ui_DMR_TextArea, -1);
  lv_obj_set_y(ui_DMR_TextArea, -90);
  lv_obj_set_align(ui_DMR_TextArea, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_DMR_TextArea, "Mac address...");
  lv_textarea_set_one_line(ui_DMR_TextArea, true);
  lv_obj_add_state(ui_DMR_TextArea, LV_STATE_CHECKED);  /// States

  ui_DMR_Reg_Button = lv_btn_create(ui_DMR_MainPanel);
  lv_obj_set_width(ui_DMR_Reg_Button, 100);
  lv_obj_set_height(ui_DMR_Reg_Button, 30);
  lv_obj_set_x(ui_DMR_Reg_Button, -100);
  lv_obj_set_y(ui_DMR_Reg_Button, -40);
  lv_obj_set_align(ui_DMR_Reg_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DMR_Reg_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DMR_Reg_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DMR_Reg_Button, lv_color_hex(0x11EF39), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DMR_Reg_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DMR_Reg_Button, lv_color_hex(0xDBF2DF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_DMR_Reg_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_DMR_Reg_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_DMR_Reg_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_DMR_Reg_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_DMR_Reg_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_DMR_Reg_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_Reg_Label = lv_label_create(ui_DMR_Reg_Button);
  lv_obj_set_width(ui_DMR_Reg_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_DMR_Reg_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_DMR_Reg_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DMR_Reg_Label, "Reg.");

  ui_DMR_Exit_Button = lv_btn_create(ui_DMR_MainPanel);
  lv_obj_set_width(ui_DMR_Exit_Button, 100);
  lv_obj_set_height(ui_DMR_Exit_Button, 30);
  lv_obj_set_x(ui_DMR_Exit_Button, 100);
  lv_obj_set_y(ui_DMR_Exit_Button, -40);
  lv_obj_set_align(ui_DMR_Exit_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DMR_Exit_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DMR_Exit_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DMR_Exit_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DMR_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DMR_Exit_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_opa(ui_DMR_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_width(ui_DMR_Exit_Button, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_color(ui_DMR_Exit_Button, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_opa(ui_DMR_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_width(ui_DMR_Exit_Button, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_shadow_spread(ui_DMR_Exit_Button, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_Exit_Label = lv_label_create(ui_DMR_Exit_Button);
  lv_obj_set_width(ui_DMR_Exit_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_DMR_Exit_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_DMR_Exit_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DMR_Exit_Label, "EXIT");
  lv_obj_set_style_text_color(ui_DMR_Exit_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DMR_Exit_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_keyboard_set_textarea(ui_DMR_Keyboard, ui_DMR_TextArea);
  lv_obj_add_event_cb(ui_DMR_Exit_Button, ui_event_DMR_Exit_Button, LV_EVENT_ALL, NULL);
}

void time_timer_cb(lv_timer_t *timer) {
  time_t t = time(NULL);
  struct tm *local = localtime(&t);

  sprintf(timeString, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
  sprintf(dateString, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon, local->tm_mday);

  //  sprintf(dateString, "%s/%s %02d %04d", DAY[local->tm_wday], MONTH[local->tm_mon], local->tm_mday,
  //          local->tm_year + 1900);

  lv_label_set_text(ui_Screen1TimeLabel, timeString);
  lv_label_set_text(ui_Screen1DateLabel, dateString);
}

void ui_init(void) {
  lv_disp_t *dispp = lv_disp_get_default();
  lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                            false, LV_FONT_DEFAULT);
  lv_disp_set_theme(dispp, theme);

  ui_Main_screen_init();
  ui_Setting_screen_init();
  ui_SettingSelect_screen_init();
  ui_DeviceManager_screen_init();
  ui_DeviceManagerReg_screen_init();

  // registers message subscriber to handle update the lvgl gui components
  // MSG_TIME_SYNCED - Update the buttons when receiving a time info from the main node
  lv_msg_subsribe(MSG_TIME_SYNCED, status_change_cb, NULL);
  lv_msg_subsribe(MSG_BATTERY_STATUS, status_change_cb, NULL);
  // MSG_IRRIGATION_STATUS - start(zone_id), stop(zone_id, flow_value)
  lv_msg_subsribe(MSG_IRRIGATION_STATUS, status_change_cb, NULL);

  lv_style_init(&style_clock);
  static uint32_t user_data = 10;
  lv_timer_t *time_timer = lv_timer_create(time_timer_cb, 1, &user_data);
  // Disable all buttons in Main screen until applying master's time.
  // disable_buttons();

  lv_disp_load_scr(ui_Main);
}
