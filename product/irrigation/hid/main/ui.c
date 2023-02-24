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
#include "log.h"
#include "sysfile.h"
#include "filelog.h"
#include "sys_config.h"

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
lv_obj_t *ui_StartButton;
lv_obj_t *ui_StartButtonLabel;
lv_obj_t *ui_StopButton;
lv_obj_t *ui_StopButtonLabel;
lv_obj_t *ui_LogButton;
lv_obj_t *ui_LogButtonLabel;
lv_obj_t *ui_SettingButton;
lv_obj_t *ui_SettingButtonLabel;
lv_obj_t *ui_OperationListPanel;
lv_obj_t *ui_OperationList;
lv_obj_t *ui_Setting;
lv_obj_t *ui_SettingPanel;
lv_obj_t *ui_SettingKeyboard;
lv_obj_t *ui_SettingTitle;
lv_obj_t *ui_FlowRateText;
lv_obj_t *ui_ZoneAreaText;
lv_obj_t *ui_TimeHourText;
lv_obj_t *ui_TimeSeperate;
lv_obj_t *ui_TimeMinuteText;
lv_obj_t *ui_SettingSaveButton;
lv_obj_t *ui_SettingSaveButtonLabel;
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
lv_obj_t *ui_SettingFICLabel;
lv_obj_t *ui_SettingTimeLabel;
lv_obj_t *ui_SettingDateLabel;

// Setting Select : SS
lv_obj_t *ui_SS_screen;
lv_obj_t *ui_SS_MainPanel;
lv_obj_t *ui_SS_OpSet_Button;
lv_obj_t *ui_SS_OpSet_Label;
lv_obj_t *ui_SS_Device_mag_Button;
lv_obj_t *ui_SS_Device_mag_Label;
// Device Manager : DM
lv_obj_t *ui_DM_screen;
lv_obj_t *ui_DM_MainPanel;
lv_obj_t *ui_DM_Roller;
lv_obj_t *ui_DM_Roller_Label;
lv_obj_t *ui_DM_Add_Button;
lv_obj_t *ui_DM_Add_Label;
lv_obj_t *ui_DM_Del_Button;
lv_obj_t *ui_DM_Del_Label;
lv_obj_t *ui_DM_Exit_Button;
lv_obj_t *ui_DM_Exit_Label;
// Device Manager Register : DMR
lv_obj_t *ui_DM_screen_Dropdown;
lv_obj_t *ui_DMR_screen;
lv_obj_t *ui_DMR_MainPanel;
lv_obj_t *ui_DMR_Keyboard;
lv_obj_t *ui_DMR_TextArea;
lv_obj_t *ui_DMR_Reg_Button;
lv_obj_t *ui_DMR_Reg_Label;
lv_obj_t *ui_DMR_Exit_Button;
lv_obj_t *ui_DMR_Exit_Label;
lv_obj_t *ui_DMR_screen_Dropdown;

extern void ui_event_StartButton(lv_event_t *e);
extern void ui_event_StopButton(lv_event_t *e);
extern void ui_event_LogButton(lv_event_t *e);
extern void ui_event_SettingButton(lv_event_t *e);
extern void ui_event_FlowRateText(lv_event_t *e);
extern void ui_event_ZoneAreaText(lv_event_t *e);
extern void ui_event_TimeHourText(lv_event_t *e);
extern void ui_event_TimeMinuteText(lv_event_t *e);
extern void ui_event_SettingSaveButton(lv_event_t *e);
extern void ui_event_SettingCancelButton(lv_event_t *e);
extern void ui_event_SS_OpSet_Button(lv_event_t *e);
extern void ui_event_SS_Device_mag_Button(lv_event_t *e);
extern void ui_event_DM_Add_Button(lv_event_t *e);
extern void ui_event_DM_Del_Button(lv_event_t *e);
extern void ui_event_DM_Exit_Button(lv_event_t *e);
extern void ui_event_DM_Roller_Select(lv_event_t *e);
extern void ui_event_DM_Dropdown_Select(lv_event_t *e);
extern void ui_event_DMR_Reg_Button(lv_event_t *e);
extern void ui_event_DMR_Exit_Button(lv_event_t *e);
extern void ui_event_DMR_Dropdown_Select(lv_event_t *e);

static lv_style_t style_clock;
char timeString[9];
char dateString[30];

const char *DAY[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const char *MONTH[] = { "January", "February", "March",     "April",   "May",      "June",
                        "July",    "August",   "September", "October", "November", "December" };

static const char *TAG = "UI";

///////////////////// TEST LVGL SETTINGS ////////////////////
/**
#if LV_COLOR_DEPTH != 16
#error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif
#if LV_COLOR_16_SWAP != 0
#error "LV_COLOR_16_SWAP should be 0 to match SquareLine Studio's settings"
#endif
**/

static void status_change_cb(void *s, lv_msg_t *m) {
  LV_UNUSED(s);

  if (lvgl_acquire()) {
    unsigned int msg_id = lv_msg_get_id(m);

    switch (msg_id) {
      case MSG_TIME_SYNCED: enable_buttons(); break;
      case MSG_BATTERY_STATUS: {
        char op_msg[128] = { 0 };
        irrigation_message_t *msg = (irrigation_message_t *)lv_msg_get_payload(m);
        device_status_t *dev_stat = (device_status_t *)&msg->payload.dev_stat;
        for (int id = 1; id < 7; id++) {
          FDATA(BASE_PATH, "Zone[%d] : Battery level = [%d]", id, dev_stat->battery_level[id]);
          snprintf(op_msg, sizeof(op_msg), "Zone[%d] : Battery level = [%d]\n", id, dev_stat->battery_level[id]);
          add_operation_list(op_msg);
        }
      } break;
      case MSG_RESPONSE_STATUS: {
        irrigation_message_t *msg = (irrigation_message_t *)lv_msg_get_payload(m);
        device_status_t *dev_stat = (device_status_t *)&msg->payload.dev_stat;
        if (msg->receive_type == SET_CONFIG && msg->resp == SUCCESS) {
          LOGI(TAG, "Got SetConfig response, Call disable start button()");
          disable_start_button();
        } else if (msg->receive_type == FORCE_STOP) {
          char op_msg[128] = { 0 };
          int zone_id = dev_stat->deviceId;
          int flow_value = dev_stat->flow_value;
          LOGI(TAG, "Got Force Stop response from [%d]", zone_id);
          enable_start_button();
          if (zone_id >= 1 && zone_id <= 6) {
            set_zone_status(zone_id, false);
            set_zone_number(zone_id, false);
            set_zone_flow_value(zone_id, flow_value);
            snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] with flow[%d] at %s\n", zone_id,
                     flow_value, get_current_timestamp());
            add_operation_list(op_msg);
            FDATA(BASE_PATH, "%s", op_msg);
          }
        }
      } break;
      case MSG_IRRIGATION_STATUS: {
        char op_msg[128] = { 0 };
        irrigation_message_t *msg = (irrigation_message_t *)lv_msg_get_payload(m);
        device_status_t *dev_stat = (device_status_t *)&msg->payload.dev_stat;
        int zone_id = dev_stat->deviceId;
        if (msg->sender_type == START_FLOW) {
          LOGI(TAG, "Got Start Flow response");
          set_zone_status(zone_id, true);
          set_zone_number(zone_id, true);
          disable_start_button();
          snprintf(op_msg, sizeof(op_msg), "Start to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
          add_operation_list(op_msg);
          FDATA(BASE_PATH, "%s", op_msg);
        } else if (msg->sender_type == ZONE_COMPLETE) {
          LOGI(TAG, "Got Zone Complete response");
          int flow_value = dev_stat->flow_value;
          set_zone_status(zone_id, false);
          set_zone_number(zone_id, false);
          set_zone_flow_value(zone_id, flow_value);
          snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] with flow[%d] at %s\n", zone_id, flow_value,
                   get_current_timestamp());
          add_operation_list(op_msg);
          FDATA(BASE_PATH, "%s", op_msg);
        } else if (msg->sender_type == ALL_COMPLETE) {
          LOGI(TAG, "Got All Complete response");
          enable_start_button();
          FDATA(BASE_PATH, "%s", "All irrigation progress are complete done!!!");
        } else if (msg->sender_type == DEVICE_ERROR) {
          // Disable zone area
          for (int i = 0; i < 6; i++) {
            if (dev_stat->child_status[i]) {
              // Display the device status as error
              snprintf(op_msg, sizeof(op_msg), "Child [%d] has an error\n", i + 1);
              add_operation_list(op_msg);
              FDATA(BASE_PATH, "%s", op_msg);
            }
          }
        }
      } break;
      default: break;
    }

    lvgl_release();
  }
}

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////

///////////////////// SCREENS ////////////////////
void ui_Main_screen_init(void) {
  char flow1[20] = { 0 };
  char flow2[20] = { 0 };
  char flow3[20] = { 0 };
  char flow4[20] = { 0 };
  char flow5[20] = { 0 };
  char flow6[20] = { 0 };

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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_1, flow1, sizeof(flow1));
  lv_label_set_text(ui_ZoneFlowmeter1, flow1);
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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_2, flow2, sizeof(flow2));
  lv_label_set_text(ui_ZoneFlowmeter2, flow2);
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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_3, flow3, sizeof(flow3));
  lv_label_set_text(ui_ZoneFlowmeter3, flow3);
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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_4, flow4, sizeof(flow4));
  lv_label_set_text(ui_ZoneFlowmeter4, flow4);
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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_5, flow5, sizeof(flow5));
  lv_label_set_text(ui_ZoneFlowmeter5, flow5);
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
  // Get flow value from syscfg
  syscfg_get_flow_value(ZONE_6, flow6, sizeof(flow6));
  lv_label_set_text(ui_ZoneFlowmeter6, flow6);
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

  ui_LogButton = lv_btn_create(ui_Main);
  lv_obj_set_width(ui_LogButton, 100);
  lv_obj_set_height(ui_LogButton, 40);
  lv_obj_set_x(ui_LogButton, 175);
  lv_obj_set_y(ui_LogButton, 31);
  lv_obj_set_align(ui_LogButton, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_LogButton, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_LogButton, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_radius(ui_LogButton, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_LogButton, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_LogButton, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  // lv_obj_add_state(ui_LogButton, LV_STATE_DISABLED);
  // _ui_state_modify(ui_LogButton, _UI_MODIFY_STATE_ADD, LV_STATE_DISABLED);

  ui_LogButtonLabel = lv_label_create(ui_LogButton);
  lv_obj_set_width(ui_LogButtonLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_LogButtonLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_LogButtonLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_LogButtonLabel, "Log");
  lv_obj_set_style_text_color(ui_LogButtonLabel, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_LogButtonLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_font(ui_LogButtonLabel, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);

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
  lv_textarea_set_placeholder_text(ui_OperationList, "...");

  lv_obj_add_event_cb(ui_StartButton, ui_event_StartButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_StopButton, ui_event_StopButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_LogButton, ui_event_LogButton, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_SettingButton, ui_event_SettingButton, LV_EVENT_ALL, NULL);

  ui_Screen1FICLabel = lv_label_create(ui_Main);
  lv_obj_set_width(ui_Screen1FICLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Screen1FICLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Screen1FICLabel, -178);
  lv_obj_set_y(ui_Screen1FICLabel, -139);
  lv_obj_set_align(ui_Screen1FICLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_Screen1FICLabel, "SEED FIC");
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
  lv_obj_set_width(ui_SettingPanel, 460);
  lv_obj_set_height(ui_SettingPanel, 280);
  lv_obj_set_x(ui_SettingPanel, 0);
  lv_obj_set_y(ui_SettingPanel, 11);
  lv_obj_set_align(ui_SettingPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_SettingPanel, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_ELASTIC | LV_OBJ_FLAG_SCROLL_MOMENTUM |
                                         LV_OBJ_FLAG_SCROLL_CHAIN);  /// Flags

  ui_SettingFICLabel = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingFICLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingFICLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingFICLabel, -187);
  lv_obj_set_y(ui_SettingFICLabel, -144);
  lv_obj_set_align(ui_SettingFICLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingFICLabel, "SEED FIC");
  lv_obj_set_style_text_color(ui_SettingFICLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SettingFICLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingTimeLabel = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingTimeLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingTimeLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingTimeLabel, 175);
  lv_obj_set_y(ui_SettingTimeLabel, -144);
  lv_obj_set_align(ui_SettingTimeLabel, LV_ALIGN_CENTER);
  lv_obj_add_style(ui_SettingTimeLabel, &style_clock, 0);
  lv_label_set_text(ui_SettingTimeLabel, timeString);
  lv_label_set_long_mode(ui_SettingTimeLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(ui_SettingTimeLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SettingTimeLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingDateLabel = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingDateLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingDateLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingDateLabel, 100);
  lv_obj_set_y(ui_SettingDateLabel, -144);
  lv_obj_set_align(ui_SettingDateLabel, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingDateLabel, dateString);
  lv_obj_set_style_text_color(ui_SettingDateLabel, lv_color_hex(0xF5F1F1), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_SettingDateLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SettingKeyboard = lv_keyboard_create(ui_SettingPanel);
  lv_keyboard_set_mode(ui_SettingKeyboard, LV_KEYBOARD_MODE_NUMBER);
  lv_obj_set_width(ui_SettingKeyboard, 428);
  lv_obj_set_height(ui_SettingKeyboard, 160);
  lv_obj_set_x(ui_SettingKeyboard, -1);
  lv_obj_set_y(ui_SettingKeyboard, 55);
  lv_obj_set_align(ui_SettingKeyboard, LV_ALIGN_CENTER);

  ui_SettingTitle = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_SettingTitle, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SettingTitle, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SettingTitle, -90);
  lv_obj_set_y(ui_SettingTitle, -111);
  lv_obj_set_align(ui_SettingTitle, LV_ALIGN_CENTER);
  lv_label_set_text(ui_SettingTitle, "flow rate / time set (Hour, Min)");

  ui_FlowRateText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_FlowRateText, 65);
  lv_obj_set_height(ui_FlowRateText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_FlowRateText, -168);
  lv_obj_set_y(ui_FlowRateText, -76);
  lv_obj_set_align(ui_FlowRateText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_FlowRateText, "flow rate...");
  lv_textarea_set_one_line(ui_FlowRateText, true);

  ui_TimeHourText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeHourText, 58);
  lv_obj_set_height(ui_TimeHourText, LV_SIZE_CONTENT);  /// 70
  lv_obj_set_x(ui_TimeHourText, -90);
  lv_obj_set_y(ui_TimeHourText, -76);
  lv_obj_set_align(ui_TimeHourText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeHourText, "Hour...");
  lv_textarea_set_one_line(ui_TimeHourText, true);

  ui_TimeSeperate = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_TimeSeperate, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_TimeSeperate, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_TimeSeperate, -49);
  lv_obj_set_y(ui_TimeSeperate, -76);
  lv_obj_set_align(ui_TimeSeperate, LV_ALIGN_CENTER);
  lv_label_set_text(ui_TimeSeperate, ":");

  ui_TimeMinuteText = lv_textarea_create(ui_Setting);
  lv_obj_set_width(ui_TimeMinuteText, 58);
  lv_obj_set_height(ui_TimeMinuteText, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_TimeMinuteText, -8);
  lv_obj_set_y(ui_TimeMinuteText, -76);
  lv_obj_set_align(ui_TimeMinuteText, LV_ALIGN_CENTER);
  lv_textarea_set_placeholder_text(ui_TimeMinuteText, "Min...");
  lv_textarea_set_one_line(ui_TimeMinuteText, true);

  ui_SettingSaveButton = lv_btn_create(ui_Setting);
  lv_obj_set_width(ui_SettingSaveButton, 70);
  lv_obj_set_height(ui_SettingSaveButton, 37);
  lv_obj_set_x(ui_SettingSaveButton, 88);
  lv_obj_set_y(ui_SettingSaveButton, -76);
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
  lv_obj_set_x(ui_SettingCancelButton, 171);
  lv_obj_set_y(ui_SettingCancelButton, -76);
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
  lv_obj_set_y(ui_Zone1, -36);
  lv_obj_set_align(ui_Zone1, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone2 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone2, "2");
  lv_obj_set_width(ui_Zone2, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone2, -73);
  lv_obj_set_y(ui_Zone2, -36);
  lv_obj_set_align(ui_Zone2, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone3 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone3, "3");
  lv_obj_set_width(ui_Zone3, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone3, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone3, -20);
  lv_obj_set_y(ui_Zone3, -36);
  lv_obj_set_align(ui_Zone3, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone4 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone4, "4");
  lv_obj_set_width(ui_Zone4, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone4, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone4, 33);
  lv_obj_set_y(ui_Zone4, -36);
  lv_obj_set_align(ui_Zone4, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone4, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone5 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone5, "5");
  lv_obj_set_width(ui_Zone5, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone5, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone5, 88);
  lv_obj_set_y(ui_Zone5, -36);
  lv_obj_set_align(ui_Zone5, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone5, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_Zone6 = lv_checkbox_create(ui_Setting);
  lv_checkbox_set_text(ui_Zone6, "6");
  lv_obj_set_width(ui_Zone6, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Zone6, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Zone6, 141);
  lv_obj_set_y(ui_Zone6, -36);
  lv_obj_set_align(ui_Zone6, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Zone6, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  ui_ZoneAreaLabel = lv_label_create(ui_Setting);
  lv_obj_set_width(ui_ZoneAreaLabel, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_ZoneAreaLabel, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_ZoneAreaLabel, -173);
  lv_obj_set_y(ui_ZoneAreaLabel, -35);
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
  ui_SS_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_SS_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_SS_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_SS_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_SS_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_SS_MainPanel = lv_obj_create(ui_SS_screen);
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
  lv_label_set_text(ui_SS_OpSet_Label, "Operation Setting");
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
  ui_DM_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_DM_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_DM_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_DM_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_MainPanel = lv_obj_create(ui_DM_screen);
  lv_obj_set_width(ui_DM_MainPanel, 450);
  lv_obj_set_height(ui_DM_MainPanel, 300);
  lv_obj_set_align(ui_DM_MainPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_DM_MainPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_bg_grad_color(ui_DM_MainPanel, lv_color_hex(0x9FA6F3), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_grad_dir(ui_DM_MainPanel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Roller = lv_roller_create(ui_DM_MainPanel);
  lv_roller_set_options(ui_DM_Roller, "Select\n", LV_ROLLER_MODE_NORMAL);
  lv_obj_set_height(ui_DM_Roller, 243);
  lv_obj_set_width(ui_DM_Roller, 150);  /// 1
  lv_obj_set_x(ui_DM_Roller, 0);
  lv_obj_set_y(ui_DM_Roller, 0);
  lv_obj_set_align(ui_DM_Roller, LV_ALIGN_CENTER);

  ui_DM_Roller_Label = lv_label_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Roller_Label, 100);  /// 1
  lv_obj_set_height(ui_DM_Roller_Label, 30);  /// 1
  lv_obj_set_x(ui_DM_Roller_Label, -150);
  lv_obj_set_y(ui_DM_Roller_Label, 0);
  lv_obj_set_align(ui_DM_Roller_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_DM_Roller_Label, "....");

  ui_DM_Add_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Add_Button, 100);
  lv_obj_set_height(ui_DM_Add_Button, 50);
  lv_obj_set_x(ui_DM_Add_Button, 150);
  lv_obj_set_y(ui_DM_Add_Button, 0);
  lv_obj_set_align(ui_DM_Add_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_Add_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DM_Add_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DM_Add_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_Add_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DM_Add_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
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
  lv_label_set_text(ui_DM_Add_Label, "Add");
  lv_obj_set_style_text_color(ui_DM_Add_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DM_Add_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Del_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Del_Button, 100);
  lv_obj_set_height(ui_DM_Del_Button, 50);
  lv_obj_set_x(ui_DM_Del_Button, 150);
  lv_obj_set_y(ui_DM_Del_Button, 95);
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
  lv_label_set_text(ui_DM_Del_Label, "Delete");
  lv_obj_set_style_text_color(ui_DM_Del_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DM_Del_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_Exit_Button = lv_btn_create(ui_DM_MainPanel);
  lv_obj_set_width(ui_DM_Exit_Button, 100);
  lv_obj_set_height(ui_DM_Exit_Button, 50);
  lv_obj_set_x(ui_DM_Exit_Button, 150);
  lv_obj_set_y(ui_DM_Exit_Button, -95);
  lv_obj_set_align(ui_DM_Exit_Button, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_Exit_Button, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_DM_Exit_Button, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_DM_Exit_Button, lv_color_hex(0xC8DAD9), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DM_Exit_Button, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_border_color(ui_DM_Exit_Button, lv_color_hex(0x585454), LV_PART_MAIN | LV_STATE_DEFAULT);
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
  lv_label_set_text(ui_DM_Exit_Label, "Exit");
  lv_obj_set_style_text_color(ui_DM_Exit_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DM_Exit_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DM_screen_Dropdown = lv_dropdown_create(ui_DM_MainPanel);
  lv_dropdown_set_options(ui_DM_screen_Dropdown, "Master\nChild");
  lv_obj_set_width(ui_DM_screen_Dropdown, 100);
  lv_obj_set_height(ui_DM_screen_Dropdown, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_DM_screen_Dropdown, -150);
  lv_obj_set_y(ui_DM_screen_Dropdown, -100);
  lv_obj_set_align(ui_DM_screen_Dropdown, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DM_screen_Dropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  lv_obj_add_event_cb(ui_DM_Add_Button, ui_event_DM_Add_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DM_Del_Button, ui_event_DM_Del_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DM_Exit_Button, ui_event_DM_Exit_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DM_screen_Dropdown, ui_event_DM_Dropdown_Select, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DM_Roller, ui_event_DM_Roller_Select, LV_EVENT_ALL, NULL);
}

void ui_DeviceManagerReg_screen_init(void) {
  char removed_device_list[80] = { 0 };

  ui_DMR_screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_DMR_screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags
  lv_obj_set_style_radius(ui_DMR_screen, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_color(ui_DMR_screen, lv_color_hex(0x011245), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_bg_opa(ui_DMR_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_MainPanel = lv_obj_create(ui_DMR_screen);
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
  lv_obj_set_width(ui_DMR_TextArea, 287);
  lv_obj_set_height(ui_DMR_TextArea, LV_SIZE_CONTENT);  /// 42
  lv_obj_set_x(ui_DMR_TextArea, 64);
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
  lv_label_set_text(ui_DMR_Reg_Label, "Reg");

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
  lv_label_set_text(ui_DMR_Exit_Label, "Exit");
  lv_obj_set_style_text_color(ui_DMR_Exit_Label, lv_color_hex(0x241D1E), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_set_style_text_opa(ui_DMR_Exit_Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_DMR_screen_Dropdown = lv_dropdown_create(ui_DMR_screen);
  // Get the removed device list from the syscfg variables
  get_removed_device_list(removed_device_list, sizeof(removed_device_list));
  lv_dropdown_set_options(ui_DMR_screen_Dropdown, removed_device_list);
  lv_obj_set_width(ui_DMR_screen_Dropdown, 98);
  lv_obj_set_height(ui_DMR_screen_Dropdown, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_DMR_screen_Dropdown, -142);
  lv_obj_set_y(ui_DMR_screen_Dropdown, -93);
  lv_obj_set_align(ui_DMR_screen_Dropdown, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_DMR_screen_Dropdown, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags

  lv_keyboard_set_textarea(ui_DMR_Keyboard, ui_DMR_TextArea);
  lv_obj_add_event_cb(ui_DMR_Reg_Button, ui_event_DMR_Reg_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DMR_Exit_Button, ui_event_DMR_Exit_Button, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DMR_screen_Dropdown, ui_event_DMR_Dropdown_Select, LV_EVENT_ALL, NULL);
}

void time_timer_cb(lv_timer_t *timer) {
  time_t t = time(NULL);
  struct tm *local = localtime(&t);

  sprintf(timeString, "%02d:%02d:%02d", local->tm_hour, local->tm_min, local->tm_sec);
  sprintf(dateString, "%04d-%02d-%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);

  lv_label_set_text(ui_Screen1TimeLabel, timeString);
  lv_label_set_text(ui_Screen1DateLabel, dateString);

  lv_label_set_text(ui_SettingTimeLabel, timeString);
  lv_label_set_text(ui_SettingDateLabel, dateString);
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
  lv_msg_subsribe(MSG_RESPONSE_STATUS, status_change_cb, NULL);
  // MSG_IRRIGATION_STATUS - start(zone_id), stop(zone_id, flow_value)
  lv_msg_subsribe(MSG_IRRIGATION_STATUS, status_change_cb, NULL);

  lv_style_init(&style_clock);
  static uint32_t user_data = 10;
  lv_timer_t *time_timer = lv_timer_create(time_timer_cb, 1, &user_data);

  // Disable all buttons in Main screen until applying master's time.
  disable_buttons();

  // update the zone status in accordance with registered child devices
  // if some child devices are removed, they are displayed as disabled mode (light-gray)
  update_zones();

  lv_disp_load_scr(ui_Main);
}
