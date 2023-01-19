// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

extern lv_obj_t *ui_Main;
extern lv_obj_t *ui_MainStatusPanel;
extern lv_obj_t *ui_ZonePanel1;
extern lv_obj_t *ui_ZoneStatus1;
extern lv_obj_t *ui_ZoneFlowmeter1;
extern lv_obj_t *ui_ZoneNum1;
extern lv_obj_t *ui_ZonePanel2;
extern lv_obj_t *ui_ZoneStatus2;
extern lv_obj_t *ui_ZoneFlowmeter2;
extern lv_obj_t *ui_ZoneNum2;
extern lv_obj_t *ui_ZonePanel3;
extern lv_obj_t *ui_ZoneStatus3;
extern lv_obj_t *ui_ZoneFlowmeter3;
extern lv_obj_t *ui_ZoneNum3;
extern lv_obj_t *ui_ZonePanel4;
extern lv_obj_t *ui_ZoneStatus4;
extern lv_obj_t *ui_ZoneFlowmeter4;
extern lv_obj_t *ui_ZoneNum4;
extern lv_obj_t *ui_ZonePanel5;
extern lv_obj_t *ui_ZoneStatus5;
extern lv_obj_t *ui_ZoneFlowmeter5;
extern lv_obj_t *ui_ZoneNum5;
extern lv_obj_t *ui_ZonePanel6;
extern lv_obj_t *ui_ZoneStatus6;
extern lv_obj_t *ui_ZoneFlowmeter6;
extern lv_obj_t *ui_ZoneNum6;
void ui_event_StartButton(lv_event_t *e);
extern lv_obj_t *ui_StartButton;
extern lv_obj_t *ui_StartButtonLabel;
void ui_event_StopButton(lv_event_t *e);
extern lv_obj_t *ui_StopButton;
extern lv_obj_t *ui_StopButtonLabel;
void ui_event_ResetButton(lv_event_t *e);
extern lv_obj_t *ui_ResetButton;
extern lv_obj_t *ui_ResetButtonLabel;
void ui_event_SettingButton(lv_event_t *e);
extern lv_obj_t *ui_SettingButton;
extern lv_obj_t *ui_SettingButtonLabel;
extern lv_obj_t *ui_OperationListPanel;
extern lv_obj_t *ui_OperationList;
extern lv_obj_t *ui_Setting;
extern lv_obj_t *ui_SettingPanel;
extern lv_obj_t *ui_SettingKeyboard;
extern lv_obj_t *ui_SettingTitle;
void ui_event_FlowRateText(lv_event_t *e);
extern lv_obj_t *ui_FlowRateText;
void ui_event_ZoneAreaText(lv_event_t *e);
extern lv_obj_t *ui_ZoneAreaText;
void ui_event_TimeHourText(lv_event_t *e);
extern lv_obj_t *ui_TimeHourText;
extern lv_obj_t *ui_TimeSeperate;
void ui_event_TimeMinuteText(lv_event_t *e);
extern lv_obj_t *ui_TimeMinuteText;
void ui_event_SettingSaveButton(lv_event_t *e);
extern lv_obj_t *ui_SettingSaveButton;
extern lv_obj_t *ui_SettingSaveButtonLabel;
void ui_event_SettingCancelButton(lv_event_t *e);
extern lv_obj_t *ui_SettingCancelButton;
extern lv_obj_t *ui_SettingCancelButtonLabel;
extern lv_obj_t *ui_Zone1;
extern lv_obj_t *ui_Zone2;
extern lv_obj_t *ui_Zone3;
extern lv_obj_t *ui_Zone4;
extern lv_obj_t *ui_Zone5;
extern lv_obj_t *ui_Zone6;
extern lv_obj_t *ui_ZoneAreaLabel;

extern lv_obj_t *ui_Screen1FICLabel;
extern lv_obj_t *ui_Screen1TimeLabel;
extern lv_obj_t *ui_Screen1DateLabel;

void OnStartEvent(lv_event_t *e);
void OnStopEvent(lv_event_t *e);
void OnSettingEvent(lv_event_t *e);
void OnResetEvent(lv_event_t *e);
void OnFlowRateEvent(lv_event_t *e);
void OnTimeHourEvent(lv_event_t *e);
void OnTimeMinuteEvent(lv_event_t *e);
void OnSettingSaveEvent(lv_event_t *e);

void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
