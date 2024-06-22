#include "../ui.h"
#include "config.h"
#include "core/lv_event.h"
#include "main.h"
#include "esp_system.h"

void ui_restart_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, "MSC");
    esp_restart();
  }
}

void ui_version_screen_init(void) {
  ui_Version_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Version_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  lv_obj_t *ui_Vendor_Label = lv_label_create(ui_Version_Screen);
  lv_obj_set_width(ui_Vendor_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Vendor_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_align(ui_Vendor_Label, LV_ALIGN_CENTER, 0, -50);
  lv_label_set_text(ui_Vendor_Label, "MS_SYSTERM");
  lv_obj_set_style_text_font(ui_Vendor_Label, &NanumBar32, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Version_Label = lv_label_create(ui_Version_Screen);
  lv_obj_set_width(ui_Version_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Version_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_align(ui_Version_Label, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(ui_Version_Label, VERSION);
  lv_obj_set_style_text_font(ui_Version_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_curr_mode_Label = lv_label_create(ui_Version_Screen);
  lv_obj_set_width(ui_curr_mode_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_curr_mode_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_align(ui_curr_mode_Label, LV_ALIGN_CENTER, 0, 50);
  lv_label_set_text(ui_curr_mode_Label, usb_mode);
  lv_obj_set_style_text_font(ui_curr_mode_Label, &NanumBar32, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_restart_Btn = lv_btn_create(ui_Version_Screen);
  lv_obj_set_width(ui_restart_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_restart_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_restart_Btn, 150);
  lv_obj_set_y(ui_restart_Btn, 85);
  lv_obj_set_align(ui_restart_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_restart_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_restart_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_restart_Btn, lv_color_hex(0x1d1d1d), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_add_event_cb(ui_restart_Btn, ui_restart_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_restart_Btn_Label = lv_label_create(ui_restart_Btn);
  lv_obj_set_width(ui_restart_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_restart_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_restart_Btn_Label, -3);
  lv_obj_set_y(ui_restart_Btn_Label, 1);
  lv_label_set_text(ui_restart_Btn_Label, "재시작");
  lv_obj_set_style_text_font(ui_restart_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}
