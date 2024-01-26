#include "../ui.h"
#include "log.h"

static const char *TAG = "ui_main_Screen";
bool device_id_check_flag = true;

static void mbox_device_id_check_event_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  LOGI(TAG, "Button %s clicked", lv_msgbox_get_active_btn_text(obj));
  if (strcmp(lv_msgbox_get_active_btn_text(obj), "Close") == 0) {
    lv_msgbox_close(obj);
  }
}

void ui_MainScreen_Btn1_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    ui_data_ctx.curr_mode = MODE_1;
    if (device_id_check_flag) {
      _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen1_screen_init);
    } else {
      static const char *btns[] = { "Close", "" };
      lv_obj_t *mbox_device_id_check = lv_msgbox_create(NULL, "Oops!", "Plase your register device ID ", btns, true);
      lv_obj_add_event_cb(mbox_device_id_check, mbox_device_id_check_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
      lv_obj_center(mbox_device_id_check);
    }
  }
}

void ui_MainScreen_Btn2_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    ui_data_ctx.curr_mode = MODE_2;
    _ui_screen_change(&ui_mode_2_scr, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_mode_2_screen_init);
  }
}

void ui_main_scr_Device_Id_Area_Label_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  char *s_device_id[10] = { 0 };
  if (event_code == LV_EVENT_READY) {
    syscfg_get(CFG_DATA, "DEVICE_ID", s_device_id, sizeof(s_device_id));
    lv_label_set_text(target, s_device_id);
  }
}

void ui_main_scr_Device_Id_Register_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    _ui_screen_change(&ui_device_id_reg_screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_device_id_reg_screen_init);
  }
}

void ui_main_screen_init(void) {
  char *s_device_id[50] = { 0 };
  int ret;
  ret = syscfg_get(CFG_DATA, "DEVICE_ID", s_device_id, sizeof(s_device_id));
  if (ret) {
    LOGI(TAG, "device id to register fail ");
    snprintf(s_device_id, sizeof(s_device_id), "NONE");
    device_id_check_flag = false;
  }

  ui_Main_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Main_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_MainScreenPanel = lv_obj_create(ui_Main_Screen);
  lv_obj_set_width(ui_MainScreenPanel, 350);
  lv_obj_set_height(ui_MainScreenPanel, 260);
  lv_obj_set_x(ui_MainScreenPanel, 0);
  lv_obj_set_y(ui_MainScreenPanel, 1);
  lv_obj_set_align(ui_MainScreenPanel, LV_ALIGN_CENTER);
  lv_obj_clear_flag(ui_MainScreenPanel, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  ui_MainScreenBtn1 = lv_btn_create(ui_MainScreenPanel);
  lv_obj_set_width(ui_MainScreenBtn1, 200);
  lv_obj_set_height(ui_MainScreenBtn1, 50);
  lv_obj_set_x(ui_MainScreenBtn1, 0);
  lv_obj_set_y(ui_MainScreenBtn1, 0);
  lv_obj_set_align(ui_MainScreenBtn1, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_MainScreenBtn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_MainScreenBtn1, LV_OBJ_FLAG_SCROLLABLE);     /// Flags

  ui_MainScreenBtn1Label = lv_label_create(ui_MainScreenBtn1);
  lv_obj_set_width(ui_MainScreenBtn1Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_MainScreenBtn1Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_MainScreenBtn1Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_MainScreenBtn1Label, "판정모드");
  lv_obj_set_style_text_font(ui_MainScreenBtn1Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_MainScreenBtn1, ui_MainScreen_Btn1_e_handler, LV_EVENT_ALL, NULL);

  ui_MainScreenBtn2 = lv_btn_create(ui_MainScreenPanel);
  lv_obj_set_width(ui_MainScreenBtn2, 200);
  lv_obj_set_height(ui_MainScreenBtn2, 50);
  lv_obj_set_x(ui_MainScreenBtn2, 0);
  lv_obj_set_y(ui_MainScreenBtn2, 70);
  lv_obj_set_align(ui_MainScreenBtn2, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_MainScreenBtn2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_MainScreenBtn2, LV_OBJ_FLAG_SCROLLABLE);     /// Flags

  lv_obj_add_event_cb(ui_MainScreenBtn2, ui_MainScreen_Btn2_e_handler, LV_EVENT_ALL, NULL);

  ui_MainScreenBtn1Label2 = lv_label_create(ui_MainScreenBtn2);
  lv_obj_set_width(ui_MainScreenBtn1Label2, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_MainScreenBtn1Label2, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_align(ui_MainScreenBtn1Label2, LV_ALIGN_CENTER);
  lv_label_set_text(ui_MainScreenBtn1Label2, "단위판정모드");
  lv_obj_set_style_text_font(ui_MainScreenBtn1Label2, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  ui_main_scr_Device_Id_Area_Label = lv_label_create(ui_MainScreenPanel);
  lv_obj_set_width(ui_main_scr_Device_Id_Area_Label, 200);  /// 1
  lv_obj_set_height(ui_main_scr_Device_Id_Area_Label, 50);  /// 1
  lv_obj_set_x(ui_main_scr_Device_Id_Area_Label, 20);
  lv_obj_set_y(ui_main_scr_Device_Id_Area_Label, -63);
  lv_obj_set_align(ui_main_scr_Device_Id_Area_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_main_scr_Device_Id_Area_Label, s_device_id);
  lv_obj_set_style_text_font(ui_main_scr_Device_Id_Area_Label, &NanumBar32, LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_main_scr_Device_Id_Area_Label, ui_main_scr_Device_Id_Area_Label_e_handler, LV_EVENT_READY,
                      NULL);

  lv_obj_t *ui_main_scr_Device_Id_Label = lv_label_create(ui_MainScreenPanel);
  lv_obj_set_width(ui_main_scr_Device_Id_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_main_scr_Device_Id_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_main_scr_Device_Id_Label, -130);
  lv_obj_set_y(ui_main_scr_Device_Id_Label, -70);
  lv_obj_set_align(ui_main_scr_Device_Id_Label, LV_ALIGN_CENTER);
  lv_label_set_text(ui_main_scr_Device_Id_Label, "ID :");
  lv_obj_set_style_text_font(ui_main_scr_Device_Id_Label, &NanumBar32, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_main_scr_Device_Id_Register_Btn = lv_btn_create(ui_MainScreenPanel);
  lv_obj_set_width(ui_main_scr_Device_Id_Register_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_main_scr_Device_Id_Register_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_main_scr_Device_Id_Register_Btn, 120);
  lv_obj_set_y(ui_main_scr_Device_Id_Register_Btn, -70);
  lv_obj_set_align(ui_main_scr_Device_Id_Register_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_main_scr_Device_Id_Register_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_main_scr_Device_Id_Register_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_main_scr_Device_Id_Register_Btn, lv_color_hex(0x0079ff),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_main_scr_Device_Id_Register_Btn, ui_main_scr_Device_Id_Register_Btn_e_hendler, LV_EVENT_ALL,
                      NULL);

  lv_obj_t *ui_main_scr_Device_Id_Register_Btn_Label = lv_label_create(ui_main_scr_Device_Id_Register_Btn);
  lv_obj_set_width(ui_main_scr_Device_Id_Register_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_main_scr_Device_Id_Register_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_main_scr_Device_Id_Register_Btn_Label, -3);
  lv_obj_set_y(ui_main_scr_Device_Id_Register_Btn_Label, 1);
  lv_label_set_text(ui_main_scr_Device_Id_Register_Btn_Label, "등 록");
  lv_obj_set_style_text_font(ui_main_scr_Device_Id_Register_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}
