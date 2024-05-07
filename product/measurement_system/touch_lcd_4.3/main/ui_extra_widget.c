
#include "stdio.h"
#include "string.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui/ui.h"
#include "wifiscan.h"

#define LV_WIDTH_RES 590
#define LV_HIGH_RES 380

static const char* TAG = "ui_extra_widget";

extern bool lvgl_acquire(void);
extern void lvgl_release(void);
extern void wifi_scan_done(void);

static void ui_wifi_ssid_list(void);
static void ta_event_cb(lv_event_t* e);
static void ui_wifi_passwd(void);

static lv_obj_t* list1 = NULL;
static lv_obj_t* kb = NULL;
static lv_obj_t* pw_panel = NULL;
static lv_obj_t* selected_ssid = NULL;

static void selected_scan_ssid_e_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    ESP_LOGI(TAG, "Clicked: %s", lv_list_get_btn_text(list1, obj));
    char* ssid = (char*)calloc(0x00, sizeof(char) * MAX_SSID);
    ssid = strtok(lv_list_get_btn_text(list1, obj), " ");
    ESP_LOGI(TAG, "strtok_ssid :  %s", ssid);
    memcpy(wifi_config_data.current_ssid, ssid, sizeof(wifi_config_data.current_ssid) - 1);
    ui_wifi_passwd();
    lv_label_set_text(selected_ssid, lv_list_get_btn_text(list1, obj));
    free(ssid);
  }
}

static void wifi_scan_btn_e_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_CLICKED) {
    ESP_LOGI(TAG, "Clicked");
    // wifi_scan_done();
    // ui_wifi_ssid_list();
    xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_SCAN);

  } else if (code == LV_EVENT_VALUE_CHANGED) {
    ESP_LOGI(TAG, "Toggled");
    // networkConnector();  // Connect to wifi using Connect btn.
  }
}

void ui_wifi_passwd(void) {
  pw_panel = lv_obj_create(ui_mainpanel3);
  lv_obj_set_size(pw_panel, 300, 200);
  lv_obj_align(pw_panel, LV_ALIGN_CENTER, 0, -70);

  /*Create a label and position it above the text box*/
  selected_ssid = lv_label_create(pw_panel);
  lv_obj_align(selected_ssid, LV_ALIGN_LEFT_MID, 0, -70);

  /*Create the password box*/
  lv_obj_t* pwd_ta = lv_textarea_create(pw_panel);
  lv_textarea_set_text(pwd_ta, "");
  lv_textarea_set_password_mode(pwd_ta, true);
  lv_textarea_set_one_line(pwd_ta, true);
  lv_obj_set_width(pwd_ta, lv_pct(80));
  lv_obj_align(pwd_ta, LV_ALIGN_LEFT_MID, 0, -10);
  lv_obj_add_event_cb(pwd_ta, ta_event_cb, LV_EVENT_ALL, NULL);

  /*Create a label and position it above the text box*/
  lv_obj_t* pwd_label = lv_label_create(pw_panel);
  lv_label_set_text(pwd_label, "Password:");
  lv_obj_align(pwd_label, LV_ALIGN_LEFT_MID, 0, -45);

  /*Create a keyboard*/
  kb = lv_keyboard_create(ui_mainpanel3);
  lv_obj_set_size(kb, LV_WIDTH_RES, LV_HIGH_RES / 2);
  lv_keyboard_set_textarea(kb, pwd_ta); /*Focus it on one of the text areas to start*/
}

static void ta_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* ta = lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    /*Focus on the clicked text area*/
    if (kb != NULL)
      lv_keyboard_set_textarea(kb, ta);
  }

  else if (code == LV_EVENT_READY) {
    ESP_LOGI(TAG, "Ready, current text: %s", lv_textarea_get_text(ta));
    memcpy(wifi_config_data.current_pwd, lv_textarea_get_text(ta), sizeof(wifi_config_data.current_pwd) - 1);
    lv_obj_del(pw_panel);
    lv_obj_del(kb);
    ESP_LOGI(TAG, "[ssid : %s], [pw : %s]", wifi_config_data.current_ssid, wifi_config_data.current_pwd);
  }
}

void wifi_scan_btn(void) {
  lv_obj_t* label;

  lv_obj_t* btn1 = lv_btn_create(ui_mainpanel3);
  lv_obj_add_event_cb(btn1, wifi_scan_btn_e_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, -200, -40);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "Scan");
  lv_obj_center(label);

  lv_obj_t* btn2 = lv_btn_create(ui_mainpanel3);
  lv_obj_add_event_cb(btn2, wifi_scan_btn_e_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, -200, 40);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Connect");
  lv_obj_center(label);
}

void ui_wifi_ssid_list(void) {
  /*Create a list*/

  char wifi_ap_str_buf[100] = { 0 };

  list1 = lv_list_create(ui_mainpanel3);
  lv_obj_set_size(list1, 300, 220);
  lv_obj_center(list1);

  /*Add buttons to the list*/
  lv_obj_t* btn;

  lv_list_add_text(list1, "Wifi_list");

  for (int i = 0; i < wifi_config_data.apCount; i++) {
    memset(wifi_ap_str_buf, 0x00, sizeof(wifi_ap_str_buf));
    snprintf(wifi_ap_str_buf, sizeof(wifi_ap_str_buf), "%s (%d) ", wifi_config_data.ap_info_list[i].ssid,
             wifi_config_data.ap_info_list[i].rssi);
    ESP_LOGI(TAG, "Scan wifi ap : %s", wifi_ap_str_buf);

    btn = lv_list_add_btn(list1, LV_SYMBOL_WIFI, wifi_ap_str_buf);
    lv_obj_add_event_cb(btn, selected_scan_ssid_e_handler, LV_EVENT_CLICKED, NULL);
  }
  free(wifi_config_data.ap_info_list);  // 메모리 해제.
}
