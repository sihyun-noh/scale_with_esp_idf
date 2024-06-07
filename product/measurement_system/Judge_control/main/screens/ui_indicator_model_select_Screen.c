#include "../ui.h"
#include "log.h"
#include "syscfg.h"
#include "scale_read_485.h"
#include "esp_system.h"
#include "main.h"

static const char *TAG = "ui_indicator_model_select_screen";

extern custom_msg_box_t *user_data_obj;
extern void Msg_Box_No_Btn_e_handler(lv_event_t *e);
extern void memory_allocation_manger();
extern void create_custom_msg_box(const char *msg_text, lv_obj_t *active_screen, void (*event_handler)(lv_event_t *),
                                  lv_event_code_t event);

lv_obj_t *ui_Indicator_Model_Select_Screen_Comfirm_Btn;
lv_obj_t *ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label;

typedef enum {
  STATE_CHANGE_SPK = 0x01,
  STATE_CHANGE_USB,
} state_change_id_t;

typedef struct state_change {
  state_change_id_t id;
  bool usb_state_change_flag;
  char before_change[5];
  char after_change[5];
} state_change_t;

state_change_t spk_state;
state_change_t usb_state;

static void mbox_close_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_current_target(e);
  LOGI(TAG, "Button %s clicked", lv_msgbox_get_active_btn_text(obj));
  if (strcmp(lv_msgbox_get_active_btn_text(obj), "Close") == 0) {
    lv_msgbox_close(obj);
  }
}

static void event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    LV_UNUSED(obj);
    char str_buf[10] = { 0 };
    strncpy(str_buf, lv_list_get_btn_text(indicator_list, obj), sizeof(str_buf));
    LOGI(TAG, "Clicked: %s", str_buf);
    syscfg_set(SYSCFG_I_INDICATOR_SET, SYSCFG_N_INDICATOR_SET, str_buf);
    if (strncmp(str_buf, "BX11", 4) == 0) {
      create_custom_msg_box("선택된 모델은 \nBX11 입니다.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);

    } else if (strncmp(str_buf, "WTM-500", 7) == 0) {
      create_custom_msg_box("선택된 모델은 \nWTM-500 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);

    } else if (strncmp(str_buf, "NT-301A", 7) == 0) {
      create_custom_msg_box(":선택된 모델은 \nNT-301A 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);

    } else if (strncmp(str_buf, "EC-D", 4) == 0) {
      create_custom_msg_box("선택된 모델은 \nEC-D Serise 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);
    } else if (strncmp(str_buf, "CB-12K", 6) == 0) {
      create_custom_msg_box("선택된 모델은 \nCB-12K 입니다.", ui_Indicator_Model_Select_Screen, NULL,
                            LV_EVENT_CLICKED);

    } else if (strncmp(str_buf, "none", 4) == 0) {
      create_custom_msg_box("다시 선택해 주십시오.", ui_Indicator_Model_Select_Screen, NULL, LV_EVENT_CLICKED);
    }
  }
}

static void Speaker_Btn_Yes_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (strncmp(speaker_set, "ON", 2) == 0) {
      syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, "OFF");
    } else if (strncmp(speaker_set, "OFF", 3) == 0) {
      syscfg_set(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, "ON");
    }
    // 다시 원 상태로 돌아온다면?
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    LOGI(TAG, "Miss loop Judge_Cancel_Btn_Yes_event_cb ");
    memory_allocation_manger();
  }
}

static void OTA_Mode_Btn_Yes_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    if (strncmp(usb_mode, "OTA", 3) == 0) {
      syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, "MSC");
    } else if (strncmp(usb_mode, "MSC", 3) == 0) {
      syscfg_set(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, "OTA");
    }
    lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_palette_main(LV_PALETTE_LIGHT_GREEN),
                              LV_PART_MAIN);
    memory_allocation_manger();
  } else {
    LOGI(TAG, "Miss loop Judge_Cancel_Btn_Yes_event_cb ");
    memory_allocation_manger();
  }
}

static void ui_Indicator_Model_Speaker_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  char s_buf[100] = { 0 };
  if (code == LV_EVENT_CLICKED) {
    LOGE(TAG, "Speaker Set");
    syscfg_get(SYSCFG_I_SPEAKER, SYSCFG_N_SPEAKER, speaker_set, sizeof(speaker_set));
    snprintf(s_buf, sizeof(s_buf), "음성지원을 사용하시겠습니까?\n현재모드:%s", speaker_set);
    LOGE(TAG, "%s", s_buf);
    create_custom_msg_box(s_buf, ui_Indicator_Model_Select_Screen, Speaker_Btn_Yes_event_cb, LV_EVENT_CLICKED);
  }
}

static void ui_Indicator_Model_OTA_MODE_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  char s_buf[100] = { 0 };
  if (code == LV_EVENT_CLICKED) {
    LOGE(TAG, "OTA_MODE");
    syscfg_get(SYSCFG_I_USB_MODE, SYSCFG_N_USB_MODE, usb_mode, sizeof(usb_mode));
    snprintf(s_buf, sizeof(s_buf), "USB MODE를 변경할까요?\n현재모드:%s", usb_mode);
    LOGE(TAG, "%s", s_buf);
    create_custom_msg_box(s_buf, ui_Indicator_Model_Select_Screen, OTA_Mode_Btn_Yes_event_cb, LV_EVENT_CLICKED);
  }
}

static void ui_Indicator_Model_Select_Screen_back_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    _ui_screen_change(&ui_Main_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, &ui_main_screen_init);
    lv_obj_clean(ui_Indicator_Model_Select_Screen);  // Function execution to prevent memory leaks
  }
}

static void mbox_close_reboot_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    esp_restart();
  }
}

static void ui_Indicator_Model_Select_Screen_Comfirm_Btn_e_hendler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    //_ui_screen_change(&ui_Main_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 100, 100, &ui_main_screen_init);
    create_custom_msg_box("재시작 하시겠습니까?", ui_Indicator_Model_Select_Screen, mbox_close_reboot_cb,
                          LV_EVENT_CLICKED);
  }
}

// static void ui_SPK_Set_State_Label_e_hendler(lv_event_t *e) {
//   lv_obj_t *target = lv_event_get_target(e);
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code = LV_EVENT_READY) {
//     lv_label_set_text(target, "음성:ON");
//   }
// }

// static void ui_USB_Set_State_Label_e_hendler(lv_event_t *e) {
//   lv_obj_t *target = lv_event_get_target(e);
//   lv_event_code_t code = lv_event_get_code(e);
//   if (code = LV_EVENT_READY) {
//     lv_label_set_text(target, "USB:MSC");
//   }
// }

void ui_indicator_model_select_screen_init(void) {
  /*Create a list*/
  ui_Indicator_Model_Select_Screen = lv_obj_create(NULL);
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen, LV_OBJ_FLAG_SCROLLABLE);  /// Flags

  indicator_list = lv_list_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_size(indicator_list, 200, 250);
  lv_obj_set_x(indicator_list, -10);
  lv_obj_set_y(indicator_list, 0);
  lv_obj_set_align(indicator_list, LV_ALIGN_CENTER);

  /*Add buttons to the list*/
  lv_obj_t *btn;
  lv_list_add_text(indicator_list, "CAS");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "WTM-500");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "NT-301A");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "EC-D");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(indicator_list, "OTHER");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "BX11");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "CB-12K");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_FILE, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  lv_list_add_text(indicator_list, "ETC");
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_OK, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
  btn = lv_list_add_btn(indicator_list, LV_SYMBOL_CLOSE, "none");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

  ui_Indicator_Model_Select_Screen_Comfirm_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Comfirm_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Comfirm_Btn, 90);
  lv_obj_set_align(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen_Comfirm_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Comfirm_Btn, lv_color_hex(0xff0060),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Select_Screen_Comfirm_Btn,
                      ui_Indicator_Model_Select_Screen_Comfirm_Btn_e_hendler, LV_EVENT_ALL, NULL);

  ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label = lv_label_create(ui_Indicator_Model_Select_Screen_Comfirm_Btn);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, "완 료");
  lv_obj_set_style_text_font(ui_Indicator_Model_Select_Screen_Comfirm_Btn_Label, &NanumBar24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_Select_Screen_Back_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Back_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Back_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Back_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Back_Btn, 30);
  lv_obj_set_align(ui_Indicator_Model_Select_Screen_Back_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Select_Screen_Back_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Select_Screen_Back_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Select_Screen_Back_Btn, lv_color_hex(0x0E04F8),
                            LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Select_Screen_Back_Btn, ui_Indicator_Model_Select_Screen_back_Btn_e_hendler,
                      LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_Select_Screen_Back_Btn_Label =
      lv_label_create(ui_Indicator_Model_Select_Screen_Back_Btn);
  lv_obj_set_width(ui_Indicator_Model_Select_Screen_Back_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Select_Screen_Back_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Select_Screen_Back_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Select_Screen_Back_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Select_Screen_Back_Btn_Label, "모 드");
  lv_obj_set_style_text_font(ui_Indicator_Model_Select_Screen_Back_Btn_Label, &NanumBar24,
                             LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_OTA_MODE_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_OTA_MODE_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_OTA_MODE_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_OTA_MODE_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_OTA_MODE_Btn, -30);
  lv_obj_set_align(ui_Indicator_Model_OTA_MODE_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_OTA_MODE_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_OTA_MODE_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_OTA_MODE_Btn, lv_color_hex(0x0E04F8), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_OTA_MODE_Btn, ui_Indicator_Model_OTA_MODE_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_OTA_MODE_Btn_Label = lv_label_create(ui_Indicator_Model_OTA_MODE_Btn);
  lv_obj_set_width(ui_Indicator_Model_OTA_MODE_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_OTA_MODE_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_OTA_MODE_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_OTA_MODE_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_OTA_MODE_Btn_Label, "USB");
  lv_obj_set_style_text_font(ui_Indicator_Model_OTA_MODE_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Indicator_Model_Speaker_Btn = lv_btn_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Indicator_Model_Speaker_Btn, LV_SIZE_CONTENT);
  lv_obj_set_height(ui_Indicator_Model_Speaker_Btn, LV_SIZE_CONTENT);
  lv_obj_set_x(ui_Indicator_Model_Speaker_Btn, 160);
  lv_obj_set_y(ui_Indicator_Model_Speaker_Btn, -90);
  lv_obj_set_align(ui_Indicator_Model_Speaker_Btn, LV_ALIGN_CENTER);
  lv_obj_add_flag(ui_Indicator_Model_Speaker_Btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);  /// Flags
  lv_obj_clear_flag(ui_Indicator_Model_Speaker_Btn, LV_OBJ_FLAG_SCROLLABLE);     /// Flags
  lv_obj_set_style_bg_color(ui_Indicator_Model_Speaker_Btn, lv_color_hex(0x0E04F8), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_add_event_cb(ui_Indicator_Model_Speaker_Btn, ui_Indicator_Model_Speaker_Btn_e_hendler, LV_EVENT_ALL, NULL);

  lv_obj_t *ui_Indicator_Model_Speaker_Btn_Label = lv_label_create(ui_Indicator_Model_Speaker_Btn);
  lv_obj_set_width(ui_Indicator_Model_Speaker_Btn_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Indicator_Model_Speaker_Btn_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Indicator_Model_Speaker_Btn_Label, -3);
  lv_obj_set_y(ui_Indicator_Model_Speaker_Btn_Label, 1);
  lv_label_set_text(ui_Indicator_Model_Speaker_Btn_Label, "SPK");
  lv_obj_set_style_text_font(ui_Indicator_Model_Speaker_Btn_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_Name_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_Name_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_Name_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_Name_Set_State_Label, 10);
  lv_obj_set_y(ui_Name_Set_State_Label, 20);
  lv_obj_set_align(ui_Name_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text(ui_Name_Set_State_Label, "설정상태");
  lv_obj_set_style_text_font(ui_Name_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_SPK_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_SPK_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_SPK_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_SPK_Set_State_Label, 10);
  lv_obj_set_y(ui_SPK_Set_State_Label, 70);
  lv_obj_set_align(ui_SPK_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text_fmt(ui_SPK_Set_State_Label, "음성 : %s", speaker_set);
  lv_obj_set_style_text_font(ui_SPK_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_obj_t *ui_USB_Set_State_Label = lv_label_create(ui_Indicator_Model_Select_Screen);
  lv_obj_set_width(ui_USB_Set_State_Label, LV_SIZE_CONTENT);   /// 1
  lv_obj_set_height(ui_USB_Set_State_Label, LV_SIZE_CONTENT);  /// 1
  lv_obj_set_x(ui_USB_Set_State_Label, 10);
  lv_obj_set_y(ui_USB_Set_State_Label, 120);
  lv_obj_set_align(ui_USB_Set_State_Label, LV_ALIGN_TOP_LEFT);
  lv_label_set_text_fmt(ui_USB_Set_State_Label, "USB : %s", usb_mode);
  lv_obj_set_style_text_font(ui_USB_Set_State_Label, &NanumBar24, LV_PART_MAIN | LV_STATE_DEFAULT);
}
