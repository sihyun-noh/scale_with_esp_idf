#include "../ui.h"
#include "config.h"
#include "main.h"
void ui_version_screen_init(void) {
  //   char *s_device_id[50] = { 0 };
  //   int ret;
  //   ret = syscfg_get(CFG_DATA, "DEVICE_ID", s_device_id, sizeof(s_device_id));
  //   if (ret) {
  //     LOGI(TAG, "device id to register fail ");
  //     snprintf(s_device_id, sizeof(s_device_id), "NONE");
  //     device_id_check_flag = false;
  //   }

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
}