/**
 * @file wifi_manager_impl.c
 *
 * @brief Wi-Fi Manager implementation for esp32 platform
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "wifi_manager_impl.h"

#include <string.h>
#include <stdbool.h>

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "log.h"
#include "sysevent.h"
#include "sys_status.h"
#include "syslog.h"

#define DEFAULT_SCAN_LIST_SIZE 32
#define DEFAULT_WIFI_CHANNEL 6
#define DEFAULT_MAX_STA_CONN 4

static const char *TAG = "wifi-manager";

static bool b_connected;
static bool b_wifi_init_done = false;
static bool b_wifi_started = false;
static uint16_t scan_count = 0;
static uint32_t scan_started = 0;
static uint32_t scan_timeout = 10000;

struct wifi_context {
  esp_interface_t ap_netif;
  esp_interface_t sta_netif;
  bool use_static_buffers;
};

static esp_netif_t *esp_netifs[ESP_IF_MAX] = { NULL, NULL, NULL };
static wifi_ap_record_t scan_ap_list[DEFAULT_SCAN_LIST_SIZE] = { 0 };

/* Private functions */

static long millis(void) {
  return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

static esp_netif_t *get_esp_interface_netif(esp_interface_t interface) {
  if (interface < ESP_IF_MAX) {
    return esp_netifs[interface];
  }
  return NULL;
}

static bool net_init(void) {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
#if CONFIG_IDF_TARGET_ESP32
    uint8_t mac[8];
    if (esp_efuse_mac_get_default(mac) == ESP_OK) {
      esp_base_mac_addr_set(mac);
    }
#endif
    initialized = (esp_netif_init() == ESP_OK);
    if (!initialized) {
      LOGE(TAG, "esp_netif_init failed!!!");
    }
    LOGI(TAG, "esp_netif_init() !!!");
  }
  return initialized;
}

static size_t _wifi_strncpy(char *dst, const char *src, size_t dst_len) {
  if (!dst || !src || !dst_len) {
    return 0;
  }
  size_t src_len = strlen(src);
  if (src_len >= dst_len) {
    src_len = dst_len;
  } else {
    src_len += 1;
  }
  memcpy(dst, src, src_len);
  return src_len;
}

/**
 * compare two AP configurations
 * @param lhs softap_config
 * @param rhs softap_config
 * @return equal
 */
static bool softap_config_equal(const wifi_config_t *lhs, const wifi_config_t *rhs) {
  if (strncmp((const char *)(lhs->ap.ssid), (const char *)(rhs->ap.ssid), 32) != 0) {
    return false;
  }
  if (strncmp((const char *)(lhs->ap.password), (const char *)(rhs->ap.password), 64) != 0) {
    return false;
  }
  if (lhs->ap.channel != rhs->ap.channel) {
    return false;
  }
  if (lhs->ap.authmode != rhs->ap.authmode) {
    return false;
  }
  if (lhs->ap.ssid_hidden != rhs->ap.ssid_hidden) {
    return false;
  }
  if (lhs->ap.max_connection != rhs->ap.max_connection) {
    return false;
  }
  if (lhs->ap.pairwise_cipher != rhs->ap.pairwise_cipher) {
    return false;
  }
  if (lhs->ap.ftm_responder != rhs->ap.ftm_responder) {
    return false;
  }
  return true;
}

static void wifi_softap_config(wifi_config_t *wifi_config, const char *ssid, const char *password, uint8_t channel,
                               wifi_auth_mode_t authmode, uint8_t ssid_hidden, uint8_t max_connections,
                               bool ftm_responder) {
  wifi_config->ap.channel = channel;
  wifi_config->ap.max_connection = max_connections;
  wifi_config->ap.beacon_interval = 100;
  wifi_config->ap.ssid_hidden = ssid_hidden;
  wifi_config->ap.authmode = WIFI_AUTH_OPEN;
  wifi_config->ap.ssid_len = 0;
  wifi_config->ap.ssid[0] = 0;
  wifi_config->ap.password[0] = 0;
  wifi_config->ap.ftm_responder = ftm_responder;
  if (ssid != NULL && ssid[0] != 0) {
    _wifi_strncpy((char *)wifi_config->ap.ssid, ssid, 32);
    wifi_config->ap.ssid_len = strlen(ssid);
    if (password != NULL && password[0] != 0) {
      wifi_config->ap.authmode = authmode;
      // Disable by default enabled insecure TKIP and use just CCMP.
      wifi_config->ap.pairwise_cipher = WIFI_CIPHER_TYPE_CCMP;
      _wifi_strncpy((char *)wifi_config->ap.password, password, 64);
    }
  }
}

static bool wifi_init(bool b_use_static_buffers) {
  if (!b_wifi_init_done) {
    b_wifi_init_done = true;
    if (!net_init()) {
      b_wifi_init_done = false;
      return b_wifi_init_done;
    }

    if (esp_netifs[ESP_IF_WIFI_AP] == NULL) {
      esp_netifs[ESP_IF_WIFI_AP] = esp_netif_create_default_wifi_ap();
    }
    if (esp_netifs[ESP_IF_WIFI_STA] == NULL) {
      esp_netifs[ESP_IF_WIFI_STA] = esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (!b_use_static_buffers) {
      cfg.static_tx_buf_num = 0;
      cfg.dynamic_tx_buf_num = 32;
      cfg.tx_buf_type = 1;
      cfg.cache_tx_buf_num = 4;  // can't be zero!
      cfg.static_rx_buf_num = 4;
      cfg.dynamic_rx_buf_num = 32;
    }
    esp_err_t err = esp_wifi_init(&cfg);
    if (err) {
      LOGE(TAG, "esp_wifi_init failed = %d", err);
      b_wifi_init_done = false;
      return b_wifi_init_done;
    }
    LOGI(TAG, "wifi_init()!!!");
  }
  return b_wifi_init_done;
}

static bool wifi_deinit(void) {
  if (b_wifi_init_done) {
    b_wifi_init_done = !(esp_wifi_deinit() == ESP_OK);
    LOGI(TAG, "wifi_deinit()!!!");
  }
  return !b_wifi_init_done;
}

static bool wifi_start(void) {
  if (b_wifi_started) {
    return true;
  }

  b_wifi_started = true;
  esp_err_t err = esp_wifi_start();
  if (err != ESP_OK) {
    b_wifi_started = false;
    LOGE(TAG, "WiFi started error = %d", err);
    return b_wifi_started;
  }
  LOGI(TAG, "wifi_start()!!!");
  return b_wifi_started;
}

static bool wifi_stop(void) {
  esp_err_t err;
  if (!b_wifi_started) {
    return true;
  }
  b_wifi_started = false;
  err = esp_wifi_stop();
  if (err) {
    LOGE(TAG, "Could not stop WiFi %d", err);
    b_wifi_started = true;
    return false;
  }
  LOGI(TAG, "wifi_stop()!!!");
  return wifi_deinit();
}

static int is_wifi_sta_ready(void) {
  int wait_count = 0;
  wifi_mode_t wifi_mode = WIFI_MODE_NULL;

  do {
    // if (sysevent_get(WIFI_EVENT, WIFI_EVENT_STA_START, NULL, 0) == 0) {
    esp_wifi_get_mode(&wifi_mode);
    if (wifi_mode == WIFI_MODE_STA || wifi_mode == WIFI_MODE_APSTA) {
      wait_count = 0;
      break;
    } else {
      wait_count++;
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  } while (wait_count < 3);

  return (wait_count == 0) ? 1 : 0;
}

static wifi_mode_t get_wifi_mode(void) {
  if (!b_wifi_init_done || !b_wifi_started) {
    return WIFI_MODE_NULL;
  }
  wifi_mode_t wifi_mode;
  if (esp_wifi_get_mode(&wifi_mode) != ESP_OK) {
    LOGE(TAG, "WiFi not started...");
    return WIFI_MODE_NULL;
  }
  return wifi_mode;
}

static bool set_wifi_mode(wifi_mode_t mode) {
  wifi_mode_t curr_wifi_mode = get_wifi_mode();

  LOGI(TAG, "set_wifi_mode: curr_mode = %d, set_mode = %d", curr_wifi_mode, mode);

  if (curr_wifi_mode == mode) {
    return true;
  }

  if (!curr_wifi_mode && mode) {
    LOGI(TAG, "Call wifi_init()!");
    if (!wifi_init(false)) {
      return false;
    }
  } else if (curr_wifi_mode && !mode) {
    LOGI(TAG, "Call wifi_stop()!");
    return wifi_stop();
  }

  esp_err_t err;
  err = esp_wifi_set_mode(mode);
  if (err) {
    LOGE(TAG, "Could not set wifi mode = %d", err);
    return false;
  }

  LOGI(TAG, "Call wifi_start()!");
  if (!wifi_start()) {
    return false;
  }

  return true;
}

static bool enable_ap_mode(bool enable) {
  wifi_mode_t curr_wifi_mode = get_wifi_mode();
  bool is_enabled = ((curr_wifi_mode & WIFI_MODE_AP) != 0);

  if (is_enabled != enable) {
    if (enable) {
      return set_wifi_mode((wifi_mode_t)(curr_wifi_mode | WIFI_MODE_AP));
    }
    return set_wifi_mode((wifi_mode_t)(curr_wifi_mode & (~WIFI_MODE_AP)));
  }

  return true;
}

static bool enable_sta_mode(bool enable) {
  wifi_mode_t curr_wifi_mode = get_wifi_mode();
  bool is_enabled = ((curr_wifi_mode & WIFI_MODE_STA) != 0);

  if (is_enabled != enable) {
    if (enable) {
      return set_wifi_mode((wifi_mode_t)(curr_wifi_mode | WIFI_MODE_STA));
    }
    return set_wifi_mode((wifi_mode_t)(curr_wifi_mode & (~WIFI_MODE_STA)));
  }

  return true;
}

static bool enable_ap_sta_mode(bool enable) {
  wifi_mode_t curr_wifi_mode = get_wifi_mode();
  bool is_enabled = ((curr_wifi_mode & WIFI_MODE_APSTA) != 0);

  if (is_enabled != enable) {
    if (enable) {
      return set_wifi_mode((wifi_mode_t)(curr_wifi_mode | WIFI_MODE_APSTA));
    }
    return set_wifi_mode((wifi_mode_t)(curr_wifi_mode & (~WIFI_MODE_APSTA)));
  }

  return true;
}

static uint16_t scan_done(void) {
  uint16_t scan_ap_num = 0;
  esp_wifi_scan_get_ap_num((uint16_t *)&scan_ap_num);
  LOGI(TAG, "scan_ap_num = %d", scan_ap_num);
  if (scan_ap_num) {
    scan_ap_num = (scan_ap_num > DEFAULT_SCAN_LIST_SIZE) ? DEFAULT_SCAN_LIST_SIZE : scan_ap_num;
    esp_wifi_scan_get_ap_records(&scan_ap_num, scan_ap_list);
  }
  return scan_ap_num;
}

//-----------------------------------------------------------------------------------------------
// Public functions that are exposed in generic WiFi APIs
//-----------------------------------------------------------------------------------------------

wifi_context_t *wifi_init_impl(void) {
  static wifi_context_t *ctx = NULL;

  if (ctx == NULL) {
    ctx = (wifi_context_t *)calloc(1, sizeof(struct wifi_context));
    if (ctx) {
      ctx->ap_netif = ESP_IF_WIFI_AP;
      ctx->sta_netif = ESP_IF_WIFI_STA;
      ctx->use_static_buffers = false;
    }
  }

  return ctx;
}

void wifi_deinit_impl(wifi_context_t *ctx) {
  if (ctx) {
    free(ctx);
    ctx = NULL;
  }
}

int wifi_ap_mode_impl(wifi_context_t *ctx, const char *ssid, const char *password) {
  if (ctx == NULL) {
    return -1;
  }

  if (!enable_ap_mode(true)) {
    LOGE(TAG, "Enable AP first!");
    return -1;
  }

  if (!ssid || *ssid == 0) {
    LOGE(TAG, "SSID missing!");
    return -1;
  }

  if (password && (strlen(password) > 0 && strlen(password) < 8)) {
    LOGE(TAG, "Password too short!");
    return -1;
  }

  wifi_config_t conf;
  wifi_config_t curr_conf;

  wifi_softap_config(&conf, ssid, password, DEFAULT_WIFI_CHANNEL, WIFI_AUTH_WPA2_PSK, 0, DEFAULT_MAX_STA_CONN, false);
  esp_err_t err = esp_wifi_get_config((wifi_interface_t)WIFI_IF_AP, &curr_conf);

  if (err) {
    LOGE(TAG, "Get AP config failed!");
    return -1;
  }

  if (!softap_config_equal(&conf, &curr_conf)) {
    err = esp_wifi_set_config((wifi_interface_t)WIFI_IF_AP, &conf);
    if (err) {
      LOGE(TAG, "Set AP config failed!");
      return -1;
    }
  }

  return 0;
}

int wifi_sta_mode_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  if (!enable_sta_mode(true)) {
    LOGE(TAG, "Enable STA first!");
    return -1;
  }

  // Disable power saving mode of STA.
  esp_wifi_set_ps(WIFI_PS_NONE);

  return 0;
}

int wifi_ap_sta_mode_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  if (!enable_ap_sta_mode(true)) {
    LOGE(TAG, "Enable AP_STA first");
    return -1;
  }

  // Disable power saving mode of AP_STA.
  esp_wifi_set_ps(WIFI_PS_NONE);

  return 0;
}

int wifi_stop_mode_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  bool b_wifi_stop = false;

  if (b_connected) {
    if (ESP_OK != esp_wifi_disconnect()) {
      return -1;
    }
    b_connected = false;
  }

  b_wifi_stop = set_wifi_mode(WIFI_MODE_NULL);

  return (b_wifi_stop == true) ? 0 : -1;
}

int wifi_connect_ap_impl(wifi_context_t *ctx, const char *ssid, const char *password) {
  if (ctx == NULL) {
    LOGE(TAG, "Invalid arguments");
    return -1;
  }

  if (!ssid || *ssid == 0x00 || strlen(ssid) > 32) {
    LOGE(TAG, "SSID too long or missing!");
    return -1;
  }

  if (!password || *password == 0x00 || (password && strlen(password) > 64)) {
    LOGE(TAG, "password too long or missing!");
    return -1;
  }

  if (b_connected) {
    LOGE(TAG, "already connected.");
    return -1;
  }

  int ret = -1;
  int reconnect_count = 0;

  wifi_config_t wifi_config = {
    .sta = { .ssid = "", .password = "", .threshold.authmode = WIFI_AUTH_WPA2_PSK },
  };

  snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
  snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);

  LOGI(TAG, "wifi_connect_ap: ssid:%s password:%s", ssid, password);

  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

  if (is_wifi_sta_ready()) {
    LOGI(TAG, "Wifi station mode is ready");
    esp_err_t rc = ESP_OK;
    do {
      rc = esp_wifi_connect();
      if (rc == ESP_OK) {
        ret = 0;
        break;
      } else {
        reconnect_count++;
        vTaskDelay(500);
      }
    } while (rc != ESP_OK && reconnect_count < 3);
  } else {
    LOGE(TAG, "Wifi station mode is not ready");
  }

  b_connected = true;
  return ret;
}

int wifi_disconnect_ap_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  if (!b_connected) {
    LOGE(TAG, "not connected.");
    return -1;
  }

  if (ESP_OK != esp_wifi_disconnect()) {
    SLOGE(TAG, "Failed to disconnect");
    return -1;
  }
  b_connected = false;
  return 0;
}

int wifi_scan_network_impl(wifi_context_t *ctx, bool async, bool show_hidden, bool passive, uint32_t max_ms_per_chan,
                           uint8_t channel, const char *ssid, const uint8_t *bssid) {
  int ret = WIFI_SCAN_FAILED;

  if (ctx == NULL) {
    return ret;
  }

  if (is_wifi_scanning()) {
    return WIFI_SCAN_RUNNING;
  }

  SLOGI(TAG, "WiFi AP scanning");

  memset(&scan_ap_list, 0x00, sizeof(wifi_ap_record_t) * DEFAULT_SCAN_LIST_SIZE);

  scan_timeout = max_ms_per_chan * 20;

  wifi_scan_config_t config;
  config.ssid = (uint8_t *)ssid;
  config.bssid = (uint8_t *)bssid;
  config.channel = channel;
  config.show_hidden = show_hidden;
  if (passive) {
    config.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    config.scan_time.passive = max_ms_per_chan;
  } else {
    config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    config.scan_time.active.min = 100;
    config.scan_time.active.max = max_ms_per_chan;
  }

  // if async is false, this API will block until scan is done
  if (esp_wifi_scan_start(&config, false) == ESP_OK) {
    scan_started = millis();
    if (!scan_started) {
      ++scan_started;
    }
  }

  set_wifi_scan_done(0);
  set_wifi_scanning(1);

  if (async) {
    return WIFI_SCAN_RUNNING;
  }

  if (wait_for_hw_event(STATUS_WIFI_SCAN_DONE, scan_timeout)) {
    scan_count = scan_done();
  }

  return scan_count;
}

int get_wifi_scan_status_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return WIFI_SCAN_FAILED;
  }

  if (scan_started && (millis() - scan_started) > scan_timeout) {
    set_wifi_scanning(0);
    return WIFI_SCAN_FAILED;
  }
  if (is_wifi_scan_done()) {
    return scan_count;
  }
  if (is_wifi_scanning()) {
    return WIFI_SCAN_RUNNING;
  }

  return WIFI_SCAN_FAILED;
}

wifi_ap_record_t *get_wifi_scan_list_impl(wifi_context_t *ctx, uint16_t *scan_num) {
  (void)ctx;

  *scan_num = scan_count;

  if (scan_count) {
    for (int i = 0; i < scan_count; i++) {
      LOGI(TAG, "ap info[%d].ssid = %s", i, scan_ap_list[i].ssid);
    }
    return scan_ap_list;
  }

  return NULL;
}

int wifi_get_current_mode_impl(wifi_context_t *ctx) {
  wifi_mode_t wifi_mode = WIFI_MODE_NULL;

  if (ctx == NULL) {
    return wifi_mode;
  }

  if (!b_wifi_init_done || !b_wifi_started) {
    return wifi_mode;
  }

  // mutex lock
  esp_err_t rc = esp_wifi_get_mode(&wifi_mode);
  if (rc != ESP_OK) {
    LOGI(TAG, "Failed to get current wifi mode, rc = %d", rc);
    wifi_mode = WIFI_MODE_NULL;
  }
  return wifi_mode;
}

int get_sta_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len) {
  if (ctx == NULL || ip_addr == NULL || addr_len == 0) {
    return -1;
  }
  esp_netif_t *sta_netif = get_esp_interface_netif(ctx->sta_netif);
  if (sta_netif == NULL) {
    return -1;
  }

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(sta_netif, &ip_info);

  snprintf(ip_addr, addr_len, IPSTR, IP2STR(&ip_info.ip));
  return 0;
}

int get_router_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len) {
  if (ctx == NULL || ip_addr == NULL || addr_len == 0) {
    return -1;
  }
  esp_netif_t *sta_netif = get_esp_interface_netif(ctx->sta_netif);
  if (sta_netif == NULL) {
    return -1;
  }

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(sta_netif, &ip_info);

  snprintf(ip_addr, addr_len, IPSTR, IP2STR(&ip_info.gw));
  return 0;
}

int get_ap_info_impl(wifi_context_t *ctx, ap_info_t *ap_info) {
  if (ctx == NULL || ap_info == NULL) {
    return -1;
  }

  wifi_ap_record_t ap_record = { 0 };

  esp_err_t rc = esp_wifi_sta_get_ap_info(&ap_record);
  if (rc == ESP_OK) {
    snprintf((char *)ap_info->ssid, sizeof(ap_info->ssid), "%s", ap_record.ssid);
    memcpy(&ap_info->bssid, ap_record.bssid, sizeof(ap_info->bssid));
    ap_info->rssi = ap_record.rssi;
    ap_info->primary_channel = ap_record.primary;
  }
  return (rc == ESP_OK) ? 0 : -1;
}

int wifi_espnow_mode_impl(wifi_context_t *ctx, wifi_op_mode_t wifi_op_mode) {
  if (ctx == NULL) {
    return -1;
  }

  if (wifi_op_mode == WIFI_OP_AP_STA) {
    // Set WiFi as WiFi AP+STA mode for esp-now working
    if (!enable_ap_sta_mode(true)) {
      LOGE(TAG, "Failed to enable AP+STA mode!!!");
      return -1;
    }
  } else if (wifi_op_mode == WIFI_OP_STA) {
    // Set WiFi as WiFi STA mode for esp-now working
    if (!enable_sta_mode(true)) {
      LOGE(TAG, "Failed to enable STA mode!!!");
      return -1;
    }
  }

#if (CONFIG_ESPNOW_WIFI_MODE == STATION)
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
#elif (CONFIFG_ESPNOW_WIFI_MODE == SOFTAP)
  esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR);
#endif

  return 0;
}
