#include "../ui.h"
#include "log.h"

void ui_MainScreen_Btn1_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    ui_data_ctx.curr_mode = MODE_1;
    lv_label_set_text(ui_Screen1_Amount_Value_Label, "\0");
    _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen1_screen_init);
  }
}

void ui_MainScreen_Btn2_e_handler(lv_event_t *e) {
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t *target = lv_event_get_target(e);
  if (event_code == LV_EVENT_CLICKED) {
    ui_data_ctx.curr_mode = MODE_2;
    lv_label_set_text(ui_Screen1_Prod_Num_Label, "\0");
    lv_label_set_text(ui_Screen1_Upper_Value_Label, "\0");
    lv_label_set_text(ui_Screen1_Lower_Value_Label, "\0");
    _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 100, 0, &ui_Screen1_screen_init);
  }
}

void ui_main_screen_init(void) {
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
  lv_obj_set_y(ui_MainScreenBtn1, -50);
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
  lv_obj_set_y(ui_MainScreenBtn2, 50);
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
}
