// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: ms_type

#ifndef _MS_TYPE_UI_H
#define _MS_TYPE_UI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "lvgl/lvgl.h"

#include "ui_helpers.h"
#include "components/ui_comp.h"
#include "components/ui_comp_hook.h"
#include "ui_events.h"

#include <stdio.h>
#include "sysfile.h"
#include "filelog.h"
#include "sys_status.h"
#include "syscfg.h"
#include "config.h"
#include "LIFO_stack.h"
#include "scale_read_485.h"

#define CONFIG_ZERO_TARE_SET 0
#define VOLUME_CONTIUNE 1  // 계속 음성나오게..요청사항

extern int cas_zero_command(void);
extern int cas_tare_command(char *s_data);

extern int (*weight_zero_command)(void);
extern char const log_table_index[];

typedef enum {
  NOTI_NONE,
  NOTI_INDICATOR_NOT_CONN,
  NOTI_HEAP_SIZE,
} ui_noti_event_t;

typedef struct ui_noti_ctx {
  lv_obj_t *curr_screen;
  ui_noti_event_t event;
  void (*handler)(void *);
  char *data;
} ui_noti_ctx_t;

typedef enum {
  NONE_E = 0x00,
  AMOUNT_VAL_E,
  JUDGE_COMFIRM_E,
} ui_event_ids_t;

typedef enum {
  PROD_NUM_1 = 0x01,
  PROD_NUM_2,
  PROD_NUM_3,
  PROD_NUM_4,
} prod_num_t;

typedef enum {
  MODE_1 = 0x01,
  MODE_2,
  MODE_MAIN,
} mode_select_t;

typedef struct screen_mode {
  mode_t mode;
  mode_t curr_mode;
} screen_mode_t;

typedef struct internal_data_trans {
  mode_select_t curr_mode;
  ui_event_ids_t ids;
  int *ptr[10];
  void (*fp)(lv_event_t *e);
  lv_obj_t *obj;
  struct StackNode *stack_root;
} ui_internal_data_ctx_t;

typedef struct textareas {
  lv_obj_t *ta1;
  lv_obj_t *ta2;
  lv_obj_t *ta3;
  lv_obj_t *ta4;
  // lv_obj_t *ta5;
} textareas_t;

typedef struct {
  lv_obj_t *target1;
  lv_obj_t *target2;
} custom_msg_box_t;

// SCREEN : ui_Version_Screen
void ui_version_screen_init();
extern lv_obj_t *ui_Version_Screen;

// SCREEN : ui_indicator_model_select_screen
void ui_indicator_model_select_screen_init(void);
extern lv_obj_t *ui_Indicator_Model_Select_Screen;
extern lv_obj_t *indicator_list;

// SCREEN: ui_time_set_screen
void ui_time_set_screen_init(void);
extern lv_obj_t *ui_Time_Date_Set_Screen;
extern lv_obj_t *ui_Time_Date_Set_Screen_Time_Label;
extern lv_obj_t *ui_Time_Date_Set_Screen_Date_Label;

// SCREEN: ui_judge_color_Screen
void ui_judge_color_screen_init(void);
extern lv_obj_t *ui_judge_color_Screen;
extern lv_obj_t *ui_judge_color_ScreenPanel;

// SCREEN: ui_main_Srceen
void ui_main_screen_init(void);
extern lv_obj_t *ui_Main_Screen;
extern lv_obj_t *ui_MainScreenPanel;
extern lv_obj_t *ui_MainScreenBtn1;
extern lv_obj_t *ui_MainScreenBtn1Label;
extern lv_obj_t *ui_MainScreenBtn2;
extern lv_obj_t *ui_MainScreenBtn1Label2;
extern lv_obj_t *ui_main_scr_Device_Id_Area_Label;
extern lv_obj_t *ui_main_scr_Indicator_Model_Label;
extern lv_obj_t *ui_main_scr_Time_Date_Label;

// SCREEN : ui_device_id_reg_screen
void ui_device_id_reg_screen_init(void);
extern lv_obj_t *ui_device_id_reg_screen;

// SCREEN: ui_mode_2_Srceen
void ui_mode_2_screen_init(void);
extern lv_obj_t *ui_mode_2_scr;
extern lv_obj_t *ui_mode_2_scr_Panel1;
extern lv_obj_t *ui_mode_2_scr_Panel1_Current_Weight_Label;
extern lv_obj_t *ui_mode_2_scr_Panel1_Current_Count_Label;
extern lv_obj_t *ui_mode_2_scr_Panel1_Zero_Point_Label;
extern lv_obj_t *ui_mode_2_scr_Panel1_Stable_Point_Label;
extern lv_obj_t *ui_mode_2_scr_Panel1_Tare_Point_Label;
extern lv_obj_t *ui_mode_2_scr_Panel1_Amount_Value_Label;

// SCREEN: ui_Screen1
void ui_Screen1_screen_init(void);
// void ui_Screen1_Setting_Btn_e_handler(lv_event_t *e);
// void ui_Screen1_Zero_Point_Set_Btn_e_handler(lv_event_t *e);
// void ui_Screen1_Tare_Point_Set_Btn_e_handler(lv_event_t *e);
// void ui_Screen1_Mode_Set_Btn_e_handler(lv_event_t *e);
void ui_event_Screen1_List_Select_Button(lv_event_t *e);
void ui_ListSelectScreen_List_Panel_Btn_e_handler(lv_event_t *e);
extern lv_obj_t *ui_Screen1;
extern lv_obj_t *ui_Panel1;
extern lv_obj_t *ui_Screen1_Panel1_Current_Weight_Label;
extern lv_obj_t *ui_Screen1_Panel1_Current_Count_Label;
extern lv_obj_t *ui_Screen1_Setting_Btn_Label;
extern lv_obj_t *ui_led1;
extern lv_obj_t *ui_led2;
extern lv_obj_t *ui_led3;
extern lv_obj_t *ui_Screen1_over_Label;
extern lv_obj_t *ui_Screen1_normal_Label;
extern lv_obj_t *ui_Screen1_lack_Label;
extern lv_obj_t *ui_Screen1_Setting_Btn;
extern lv_obj_t *ui_Screen1_Panel1_Zero_Point_Label;
extern lv_obj_t *ui_Screen1_Panel1_Stable_Point_Label;
extern lv_obj_t *ui_Screen1_Panel1_Tare_Point_Label;
extern lv_obj_t *ui_Screen1_Amount_Value_Label;
extern lv_obj_t *ui_Screen1_Prod_Num_Label;
extern lv_obj_t *ui_Screen1_Upper_Value_Label;
extern lv_obj_t *ui_Screen1_Lower_Value_Label;
extern lv_obj_t *ui_Screen1_Judge_Comfirm_Btn;

// extern lv_obj_t *ui_Recently_Msg_Box_Panel;
// extern lv_obj_t *ui_Recently_Msg_Box_Panel_Label;

// SCREEN: ui_Screen2
void ui_Screen2_screen_init(void);
extern lv_obj_t *ui_Screen2;

// SCREEN: ui_list_select
void ui_list_select_screen_init(void);
void ui_ListSelectScreen_Comfirm_Btn_e_handler(lv_event_t *e);
void ui_ListSelectScreen_Delete_Btn_e_handler(lv_event_t *e);
extern lv_obj_t *ui_list_select;
extern lv_obj_t *ui_ListSelectScreen_List_Panel;
extern lv_obj_t *ui_ListSelectScreen_Comfirm_Panel;
extern lv_obj_t *ui_ListSelectScreen_Comfirm_Label;
extern lv_obj_t *ui_ListSelectScreen_Comfirm_Btn;
extern lv_obj_t *ui_ListSelectScreen_Comfirm_Btn_Label;
extern lv_obj_t *ui_ListSelectScreen_Delete_Btn;
extern lv_obj_t *ui_ListSelectScreen_Delete_Btn_Label;

extern char s_tare_set_value[10];
extern float upper_weight_value;
extern float lower_weight_value;
extern int prod_num_value;
extern float renge_weight_value;
extern float amount_weight_value;
extern int mode_2_compare_count;
extern char buf_prod_name[10];
extern bool printer_state;
extern bool setting_printer_flag;
extern weight_unit_t prod_weight_unit;

extern screen_mode_t curr_mode;
extern ui_event_ids_t ui_event;
extern ui_internal_data_ctx_t ui_data_ctx;
extern ui_noti_ctx_t ui_noti;

LV_FONT_DECLARE(ui_font_Display16);
LV_FONT_DECLARE(ui_font_Display24);
LV_FONT_DECLARE(ui_font_Display40);
LV_FONT_DECLARE(ui_font_Display90);
LV_FONT_DECLARE(NanumBar16);
LV_FONT_DECLARE(NanumBar18);
LV_FONT_DECLARE(NanumBar24);
LV_FONT_DECLARE(NanumBar32);

void ui_init(void);

int get_decimal_places(float number);
const char *get_format_specifier(float number);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
