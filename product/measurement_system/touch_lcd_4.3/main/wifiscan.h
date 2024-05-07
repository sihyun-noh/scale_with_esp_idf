#ifndef __WIFI_SCAN_H__
#define __WIFI_SCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "wifi_manager.h"

#define WIFI_CONNECTED BIT0
#define WIFI_DISCONNECT BIT1

/** The event group allows multiple bits for each event,
 * but we only care about one event - are we connected to the AP with an IP? */
#define LVGL_WIFI_CONFIG_CONNECTED BIT0
#define LVGL_WIFI_CONFIG_SCAN BIT1
#define LVGL_WIFI_CONFIG_SCAN_DONE BIT2
#define LVGL_WIFI_CONFIG_CONNECT_FAIL BIT4
#define LVGL_WIFI_CONFIG_TRY_CONNECT BIT5

/**********************
 *      TYPEDEFS
 **********************/
/** @brief Description of an WiFi AP */
// typedef struct {
//   uint8_t bssid[6];          /**< MAC address of AP */
//   char ssid[33];             /**< SSID of AP */
//   uint8_t primary;           /**< channel of AP */
//   wifi_second_chan_t second; /**< secondary channel of AP */
//   int8_t rssi;               /**< signal strength of AP */
//   wifi_auth_mode_t authmode; /**< authmode of AP */
// } ap_info_t;

typedef struct {
  uint16_t apCount;
  ap_info_t *ap_info_list;
  uint16_t current_ap;
  // const char *current_ssid;
  // const char *current_pwd;
  char current_ssid[MAX_SSID];
  char current_pwd[MAX_SSID];
} wifi_config_data_t;

extern wifi_config_data_t wifi_config_data;
extern EventGroupHandle_t g_wifi_event_group;

#ifdef __cplusplus
}
#endif

#endif
