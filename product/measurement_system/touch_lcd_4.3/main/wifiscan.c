/*
  WiFi Scan Test
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "wifi_manager.h"
#include "sys_status.h"
#include "sysevent.h"
#include "wifi_manager_private.h"
#include "wifiscan.h"
#include "mqtt.h"

#include "lvgl.h"
#include "ui/ui.h"

#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE
#define LV_WIDTH_RES 590
#define LV_HIGH_RES 380

static const char* TAG = "wifi_scan_lvgl";

extern bool lvgl_acquire(void);
extern void lvgl_release(void);

static void ui_wifi_ssid_list(void);
static void ta_event_cb(lv_event_t* e);
static void ui_wifi_passwd(void);

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t g_wifi_event_group = NULL;
wifi_config_data_t wifi_config_data;

static char list_buff[10][100] = { 0 };
static char (*str_buff)[100] = list_buff;
static uint16_t ap_scan_num = 0;
static char ip_addr[16] = { 0 };

/*LVGL*/
static lv_obj_t* list1 = NULL;
static lv_obj_t* kb = NULL;
static lv_obj_t* pw_panel = NULL;
static lv_obj_t* selected_ssid = NULL;
static lv_obj_t* success_panel = NULL;

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
    ESP_LOGI(TAG, "wifi_scan_btn Clicked");
    // wifi_scan_done();
    // ui_wifi_ssid_list();
    xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_SCAN);
  }
}

static void wifi_connect_btn_e_handler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    ESP_LOGI(TAG, "wifi_connect_btn_Clicked");
    // networkConnector();  // Connect to wifi using Connect btn.
    xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_TRY_CONNECT);
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
    lv_obj_del(kb);
    lv_obj_del(pw_panel);
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
  lv_obj_add_event_cb(btn2, wifi_connect_btn_e_handler, LV_EVENT_ALL, NULL);
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

void wifi_connect_success(void) {
  success_panel = lv_obj_create(ui_mainpanel3);
  lv_obj_set_size(success_panel, 300, 200);
  lv_obj_align(success_panel, LV_ALIGN_CENTER, 0, -70);

  /*Create a label and position it above the text box*/
  lv_obj_t* success_panel_label = lv_label_create(success_panel);
  lv_label_set_text(success_panel_label, "connected IP :");
  lv_obj_align(success_panel_label, LV_ALIGN_LEFT_MID, 0, -45);
}

void wifi_scan_done() {
  ESP_LOGI(TAG, "Scan nearby AP network\n");

  wifi_config_data.apCount = wifi_scan_network(false, false, false, 500, 0, NULL, NULL);
  ESP_LOGE(TAG, "scan ap : %d", wifi_config_data.apCount);
  // uint16_t scan_ap_num = wifi_scan_network(false, false, false, 500, 1, NULL, NULL);
  if (wifi_config_data.apCount > 0) {
    wifi_config_data.ap_info_list = get_wifi_scan_list(&ap_scan_num);
    if (wifi_config_data.ap_info_list) {
      printf("scan ap num [%d], [%d]\n", wifi_config_data.apCount, ap_scan_num);
      for (int i = 0; i < ap_scan_num; i++) {
        printf("ap_info_list[%d].ssid = %s\n", i, wifi_config_data.ap_info_list[i].ssid);
        printf("ap_info_list[%d].rssi = %d\n", i, wifi_config_data.ap_info_list[i].rssi);
        memset(str_buff + 1, 0x00, sizeof(*(str_buff + i)));
        sprintf(str_buff + i, "%s (%d) ", wifi_config_data.ap_info_list[i].ssid, wifi_config_data.ap_info_list[i].rssi);
        ESP_LOGI(TAG, "%s", *(str_buff + i));
      }
      // free(wifi_config_data.ap_info_list);  // 이거 처리해야함
    }
  }
}

int sta_disconnect_handler(void* arg) {
  if (g_wifi_event_group) {
    xEventGroupSetBits(g_wifi_event_group, WIFI_DISCONNECT);
  }
  return 0;
}

int sta_ip_handler(void* arg) {
  if (g_wifi_event_group) {
    xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_CONNECTED);
  }
  return 0;
}

void wifi_config_event_task(void* pvParameters) {
  while (1) {
    EventBits_t uxBits;
    uxBits = xEventGroupWaitBits(g_wifi_event_group,
                                 (LVGL_WIFI_CONFIG_SCAN | LVGL_WIFI_CONFIG_SCAN_DONE | LVGL_WIFI_CONFIG_CONNECTED |
                                  LVGL_WIFI_CONFIG_CONNECT_FAIL | LVGL_WIFI_CONFIG_TRY_CONNECT),
                                 pdTRUE, pdFALSE, portMAX_DELAY);

    switch (uxBits) {
      case LVGL_WIFI_CONFIG_SCAN: {
        wifi_scan_done();

        xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_SCAN_DONE);
        esp_wifi_scan_stop();

        break;
      }
      case LVGL_WIFI_CONFIG_SCAN_DONE: {
        ESP_LOGI(TAG, "[ * ] running refresh wifi list：%d\n", wifi_config_data.apCount);
        ui_wifi_ssid_list();
        break;
      }
      case LVGL_WIFI_CONFIG_CONNECTED: {
        wifi_connect_success();
        // sysevent_get(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_addr, sizeof(ip_addr));
        // ESP_LOGI(TAG, "address %s", ip_addr);
        break;
      }

      case LVGL_WIFI_CONFIG_CONNECT_FAIL: {
        // esp_wifi_disconnect();
        // lvgl_acquire();
        // wifi_connect_fail();
        // lvgl_release();
        break;
      }
      case LVGL_WIFI_CONFIG_TRY_CONNECT: {
        int ret = 0;
        ret = wifi_sta_mode();
        ESP_LOGI(TAG, "wifi_sta_mode state : %d", ret);
        ret = wifi_disconnect_ap();
        ESP_LOGI(TAG, "wifi_disconnect_ap state : %d", ret);
        wifi_connect_ap(wifi_config_data.current_ssid, wifi_config_data.current_pwd);
        ESP_LOGI(TAG, "wifi_connect_ap state : %d", ret);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        sysevent_get(IP_EVENT, IP_EVENT_STA_GOT_IP, ip_addr, sizeof(ip_addr));
        ESP_LOGI(TAG, "address %s", ip_addr);
        if (g_wifi_event_group) {
          xEventGroupSetBits(g_wifi_event_group, LVGL_WIFI_CONFIG_CONNECTED);
        }

        break;
      }

      default: break;
    }
  }

  // // unsigned long startingTime = millis();
  // wifi_sta_mode();
  // wifi_disconnect_ap();
  // wifi_connect_ap(wifi_config_data.current_ssid, wifi_config_data.current_pwd);
  // EventBits_t bits =
  //     xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED | WIFI_DISCONNECT, pdFALSE, pdFALSE, portMAX_DELAY);

  // if (bits & WIFI_CONNECTED) {
  //   ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", wifi_config_data.current_ssid,
  //   wifi_config_data.current_pwd); xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED);
  //   ////////////// mqtt start //////////
  //   mqtt_app_start();
  //   ////////////// mqtt start //////////
  //   // networkStatus = NETWORK_CONNECTED_POPUP;
  // } else if (bits & WIFI_DISCONNECT) {
  //   // ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", wifi_config_data.current_ap,
  //   //        wifi_config_data.current_pwd);
  //   xEventGroupClearBits(s_wifi_event_group, WIFI_DISCONNECT);
  //   // networkStatus = NETWORK_CONNECT_FAILED;
  // } else {
  //   // ESP_LOGI(TAG, "UNEXPECTED EVENT");
  // }
  // vTaskDelete(NULL);
}

void wifi_scan_main() {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
  ESP_LOGI(TAG, "wifi init");
  wifi_user_init();
  sys_stat_init();
  sysevent_create();
  wifi_sta_mode();

  g_wifi_event_group = xEventGroupCreate();

  xTaskCreate(wifi_config_event_task, "beginWIFITask", 8192, NULL, 4, NULL);
  vTaskDelay(100 / portTICK_PERIOD_MS);

  // sysevent_get_with_handler(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_ip_handler, NULL); /*add event register*/
  sysevent_get_with_handler(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, sta_disconnect_handler, NULL);
}
