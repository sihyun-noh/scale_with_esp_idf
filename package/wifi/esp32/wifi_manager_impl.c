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
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"
#include "log.h"
#include "sysevent.h"
#include "syslog.h"

#define DEFAULT_SCAN_LIST_SIZE 20
#define DEFAULT_WIFI_CHANNEL 6
#define DEFAULT_MAX_STA_CONN 4

static const char *TAG = "wifi-manager";
static bool b_connected;
struct wifi_context {
  SemaphoreHandle_t wifi_mutex;
  esp_netif_t *ap_netif;
  esp_netif_t *sta_netif;
};

static int is_wifi_sta_ready() {
  int wait_count = 0;
  wifi_mode_t wifi_mode = WIFI_MODE_NULL;

  do {
    // if (sysevent_get(WIFI_EVENT, WIFI_EVENT_STA_START, NULL, 0) == 0) {
    esp_wifi_get_mode(&wifi_mode);
    if (wifi_mode == WIFI_MODE_STA) {
      wait_count = 0;
      break;
    } else {
      wait_count++;
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  } while (wait_count < 3);

  return (wait_count == 0) ? 1 : 0;
}

wifi_context_t *wifi_init_impl() {
  static wifi_context_t *ctx = NULL;

  if (ctx == NULL) {
    esp_err_t ret;
    ctx = (wifi_context_t *)calloc(1, sizeof(struct wifi_context));
    if (ctx == NULL) {
      goto err_ctx;
    }

    // Create mutex
    ctx->wifi_mutex = xSemaphoreCreateMutex();
    if (!ctx->wifi_mutex) {
      SLOGE(TAG, "wifi_user_init: failed to create mutex");
      goto err_wifi_mutex;
    }

    ret = esp_netif_init();
    if (ret != ESP_OK) {
      SLOGE(TAG, "Netif initializing failed");
      goto err_netif;
    }

    xSemaphoreGive(ctx->wifi_mutex);
  }

  return ctx;

err_netif:
  vSemaphoreDelete(ctx->wifi_mutex);
err_wifi_mutex:
err_ctx:
  free(ctx);
  ctx = NULL;
  return ctx;
}

void wifi_deinit_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return;
  }

  vSemaphoreDelete(ctx->wifi_mutex);
  free(ctx);
  ctx = NULL;
}

int wifi_ap_mode_impl(wifi_context_t *ctx, const char *ssid, const char *password) {
  if (ctx == NULL || ssid == NULL || password == NULL) {
    return -1;
  }

  esp_err_t ret = ESP_OK;

  // Mutex lock
  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);

  // Create netif that is related to the AP mode.
  if (ctx->ap_netif == NULL) {
    ctx->ap_netif = esp_netif_create_default_wifi_ap();
    assert(ctx->ap_netif);
  }

  // Initialize wifi driver
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to init wifi mode");
    goto _error;
  }

  // Configuration of WiFi AP.
  wifi_config_t wifi_config = {
    .ap = { .ssid = "",
            .ssid_len = 0,
            .channel = DEFAULT_WIFI_CHANNEL,
            .password = "",
            .max_connection = DEFAULT_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK },
  };

  if (strlen(password) == 0) {
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  snprintf((char *)wifi_config.ap.ssid, sizeof(wifi_config.ap.ssid), "%s", ssid);
  snprintf((char *)wifi_config.ap.password, sizeof(wifi_config.ap.password), "%s", password);
  wifi_config.ap.ssid_len = strlen((const char *)wifi_config.ap.ssid);

  // Set WiFi AP and start it
  ret = esp_wifi_set_mode(WIFI_MODE_AP);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to set WiFi AP mode");
    goto _error;
  }
  ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to set WiFi AP Config mode");
    goto _error;
  }
  ret = esp_wifi_start();

  LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", wifi_config.ap.ssid, wifi_config.ap.password,
       DEFAULT_WIFI_CHANNEL);

_error:
  // Mutex unlock
  xSemaphoreGive(ctx->wifi_mutex);
  return (ret == ESP_OK) ? 0 : -1;
}

int wifi_sta_mode_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);

  if (ctx->sta_netif == NULL) {
    ctx->sta_netif = esp_netif_create_default_wifi_sta();
    assert(ctx->sta_netif);
  }

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_err_t ret = esp_wifi_init(&cfg);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to init wifi mode");
    goto _error;
  }
  ret = esp_wifi_set_mode(WIFI_MODE_STA);
  if (ret != ESP_OK) {
    LOGE(TAG, "Failed to set WiFi STA mode");
  }

_error:
  xSemaphoreGive(ctx->wifi_mutex);
  return (ret == ESP_OK) ? 0 : -1;
}

int wifi_stop_mode_impl(wifi_context_t *ctx) {
  if (ctx == NULL) {
    return -1;
  }

  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);

  esp_err_t ret = ESP_OK;

  if (b_connected) {
    ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
      goto _error;
    }
    b_connected = false;
  }

  ret = esp_wifi_stop();
  if (ret != ESP_OK) {
    goto _error;
  }

  vTaskDelay(500 / portTICK_PERIOD_MS);

  if ((ret = esp_wifi_deinit()) != ESP_OK) {
    SLOGE(TAG, "WiFi is not initialize");
    goto _error;
  }

_error:
  xSemaphoreGive(ctx->wifi_mutex);
  return (ret == ESP_OK) ? 0 : -1;
}

int wifi_connect_ap_impl(wifi_context_t *ctx, const char *ssid, const char *password) {
  if (ctx == NULL || ssid == NULL || password == NULL) {
    LOGE(TAG, "Invalid arguments");
    return -1;
  }

  if (b_connected) {
    LOGE(TAG, "already connected.");
    return -1;
  }

  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);

  int ret = -1;
  int reconnect_count = 0;

  wifi_config_t wifi_config = {
    .sta = { .ssid = "", .password = "", .threshold.authmode = WIFI_AUTH_WPA2_PSK },
  };

  snprintf((char *)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
  snprintf((char *)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", password);

  LOGI(TAG, "wifi_connect_ap: ssid:%s password:%s", ssid, password);

  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();

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
  xSemaphoreGive(ctx->wifi_mutex);

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

  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);
  int ret = 0;

  if (ESP_OK != esp_wifi_disconnect()) {
    SLOGE(TAG, "Failed to disconnect");
    ret = -1;
  }
  b_connected = false;
  xSemaphoreGive(ctx->wifi_mutex);
  return ret;
}

int wifi_scan_network_impl(wifi_context_t *ctx, scan_network_result_t *userdata, bool block, int waitSec) {
  uint16_t scan_ap_num = DEFAULT_SCAN_LIST_SIZE;
  wifi_ap_record_t scan_ap_list[DEFAULT_SCAN_LIST_SIZE] = { 0 };

  int ret = -1;

  if (ctx == NULL) {
    return ret;
  }

  // mutex lock
  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);

  if (userdata == NULL) {
    goto scan_exit;
  }
  // Check if the userdata parameters are valid
  if (userdata->scan_ap_req_count == 0 || userdata->scan_done_handler == NULL) {
    goto scan_exit;
  }

  SLOGI(TAG, "WiFi AP scanning");

  // if block is true, this API will block until scan is done
  esp_wifi_scan_start(NULL, block);
  if (block) {
    // scan ap function is blocking mode
    esp_wifi_scan_get_ap_num((uint16_t *)&userdata->scan_ap_num);
    esp_wifi_scan_get_ap_records(&scan_ap_num, scan_ap_list);
    userdata->scan_ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * userdata->scan_ap_num);
    memcpy(userdata->scan_ap_list, scan_ap_list, sizeof(wifi_ap_record_t) * userdata->scan_ap_num);
    userdata->scan_done_handler(userdata);
  } else {
    // scan ap function is non-blocking mode
    // Wait for given some time.
  }
  ret = 0;

scan_exit:
  // mutex unlock
  xSemaphoreGive(ctx->wifi_mutex);
  return ret;
}

int wifi_get_current_mode_impl(wifi_context_t *ctx) {
  wifi_mode_t wifi_mode = WIFI_MODE_NULL;
  if (ctx == NULL) {
    return wifi_mode;
  }

  // mutex lock
  xSemaphoreTake(ctx->wifi_mutex, portMAX_DELAY);
  esp_err_t rc = esp_wifi_get_mode(&wifi_mode);
  if (rc != ESP_OK) {
    LOGI(TAG, "Failed to get current wifi mode, rc = %d", rc);
  }
  xSemaphoreGive(ctx->wifi_mutex);
  return wifi_mode;
}

int get_sta_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len) {
  if (ctx == NULL || ip_addr == NULL || addr_len == 0) {
    return -1;
  }
  if (ctx->sta_netif == NULL) {
    return -1;
  }

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(ctx->sta_netif, &ip_info);

  snprintf(ip_addr, addr_len, IPSTR, IP2STR(&ip_info.ip));
  return 0;
}

int get_router_ipaddr_impl(wifi_context_t *ctx, char *ip_addr, int addr_len) {
  if (ctx == NULL || ip_addr == NULL || addr_len == 0) {
    return -1;
  }
  if (ctx->sta_netif == NULL) {
    return -1;
  }

  esp_netif_ip_info_t ip_info;
  esp_netif_get_ip_info(ctx->sta_netif, &ip_info);

  snprintf(ip_addr, addr_len, IPSTR, IP2STR(&ip_info.gw));
  return 0;
}

int get_ap_info_impl(wifi_context_t *ctx, wifi_ap_record_t *ap_info) {
  if (ctx == NULL || ap_info == NULL) {
    return -1;
  }

  esp_err_t rc = esp_wifi_sta_get_ap_info(ap_info);
  return (rc == ESP_OK) ? 0 : -1;
}
