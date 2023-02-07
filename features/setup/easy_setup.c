/**
 * @file easy_setup.c
 *
 * @brief easy setup implementation for application layer
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "syscfg.h"
#include "sysevent.h"
#include "sys_config.h"
#include "sys_status.h"
#include "syslog.h"
#include "event_ids.h"
#include "wifi_manager.h"

#include "esp_wifi_types.h"
#include "esp_mac.h"

#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_https_server.h>

#include "time_api.h"
#include "cJSON.h"
#include "mdns.h"

#define WIFI_PASS "!ALfE42vcchYpFQyPuCN*v_w"
#define PREFIX_SSID "COMFAST"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED BIT0
#define WIFI_DISCONNECT BIT1

#define MAX_RETRY_CONNECT 5

typedef enum {
  UNCONFIGURED_MODE = 0,
  PAIRING_MODE,
  CONFIRM_MODE,
  STA_CONNECT_MODE,
  PAIRED_MODE,
  UNPAIRED_MODE,
  SERVER_GET_MODE
} setup_mode_t;

static const char *TAG = "easy_setup";

static TaskHandle_t easy_setup_handle = NULL;
static httpd_handle_t httpd_handle = NULL;

static int ap_mode_running = 0;
static int s_retry_connect = 0;

static int router_connect = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

extern void initialise_mdns(void);

char AP_SSID[30] = { 0 };
char farmssid[50] = { 0 };
char farmpw[50] = { 0 };
char farmip[30] = { 0 };

static void set_gateway_address(void) {
  char router_ip[20] = { 0 };

  get_router_ipaddr(router_ip, sizeof(router_ip));
  if (router_ip[0]) {
    syscfg_set(SYSCFG_I_GATEWAYIP, SYSCFG_N_GATEWAYIP, router_ip);
  }
}

static char *get_best_wifi_ssid(void) {
  // scan nearby AP networks
  char *ssid = NULL;
  int best_rssi = -999, best_id = -1;

  uint16_t actual_ap_num = 0;
  ap_info_t *ap_info_list = NULL;

  uint16_t scan_ap_num = wifi_scan_network(false, false, false, 500, 1, NULL, NULL);

  if (scan_ap_num > 0) {
    ap_info_list = get_wifi_scan_list(&actual_ap_num);
    if (ap_info_list) {
      LOGI(TAG, "Scan AP num = [%d], [%d]", scan_ap_num, actual_ap_num);
      for (int i = 0; i < actual_ap_num; i++) {
        LOGI(TAG, "Scan WiFi SSID = [%s], RSSI = [%d]", (char *)ap_info_list[i].ssid, ap_info_list[i].rssi);
        if (strstr((char *)ap_info_list[i].ssid, PREFIX_SSID)) {
          if (best_rssi < ap_info_list[i].rssi) {
            best_rssi = ap_info_list[i].rssi;
            best_id = i;
          }
        }
      }
      LOGI(TAG, "best_rssi = %d, best_id = %d", best_rssi, best_id);
      if (best_rssi != -999 && best_id != -1) {
        int ssid_len = strlen((char *)ap_info_list[best_id].ssid);
        ssid = calloc(1, ssid_len + 1);
        strncpy(ssid, (char *)ap_info_list[best_id].ssid, ssid_len);
        return ssid;
      }
    }
  }
  return NULL;
}

/**
 * @brief An HTTP GET handler
 *
 * @param httpd_req_t* HTTP Request Data Structure
 */
static esp_err_t root_get_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, "<h1>ZeroConfig Test</h1>", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

/**
 * @brief Our URI handler function to be called during POST /uri request
 *
 * @param httpd_req_t* HTTP Request Data Structure
 */
static esp_err_t post_handler(httpd_req_t *req) {
  /* Destination buffer for content of HTTP POST request.
   * httpd_req_recv() accepts char* only, but content could
   * as well be any binary data (needs type casting).
   * In case of string data, null termination will be absent, and
   * content length would give length of string */
  char content[100] = { 0 };

  /* Truncate if content length larger than the buffer */
  size_t recv_size = MIN(req->content_len, sizeof(content));

  int ret = httpd_req_recv(req, content, recv_size);
  if (ret <= 0) { /* 0 return value indicates connection closed */
    /* Check if timeout occurred */
    if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
      /* In case of timeout one can choose to retry calling
       * httpd_req_recv(), but to keep it simple, here we
       * respond with an HTTP 408 (Request Timeout) error */
      httpd_resp_send_408(req);
    }
    /* In case of error, returning ESP_FAIL will
     * ensure that the underlying socket is closed */
    return ESP_FAIL;
  }

  LOGI(TAG, "%s ", content);

  cJSON *root = cJSON_Parse(content);

  cJSON *ssid = cJSON_GetObjectItem(root, "SSID");
  cJSON *pw = cJSON_GetObjectItem(root, "Password");
  if (cJSON_IsString(ssid) && cJSON_IsString(pw)) {
    // ssid pw
    snprintf(farmssid, sizeof(farmssid), "%s", ssid->valuestring);
    snprintf(farmpw, sizeof(farmpw), "%s", pw->valuestring);
    syscfg_set(SYSCFG_I_SSID, SYSCFG_N_SSID, farmssid);
    syscfg_set(SYSCFG_I_PASSWORD, SYSCFG_N_PASSWORD, farmpw);
    LOGI(TAG, "Got SSID = [%s] password = [%s]", farmssid, farmpw);
  }

  cJSON *Server = cJSON_GetObjectItem(root, "Server");
  if (cJSON_IsString(Server)) {
    snprintf(farmip, sizeof(farmip), "%s", Server->valuestring);
    syscfg_set(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip);
    LOGI(TAG, "Got server url = %s ", Server->valuestring);
  }

  cJSON_Delete(root);

  /* Send a simple response */
  const char resp[] = "Server Response OK";
  httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

static const httpd_uri_t root = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler };
static const httpd_uri_t uri_post = { .uri = "/", .method = HTTP_POST, .handler = post_handler, .user_ctx = NULL };

/**
 * @brief https start webserver
 *
 */
static httpd_handle_t start_webserver(void) {
  httpd_handle_t server = NULL;

  // Start the httpd server
  LOGI(TAG, "Starting server");

  httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

  conf.port_secure = 30001;  // 30001 Port

  extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
  extern const unsigned char cacert_pem_end[] asm("_binary_cacert_pem_end");
  conf.servercert = cacert_pem_start;
  conf.servercert_len = cacert_pem_end - cacert_pem_start;

  extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
  extern const unsigned char prvtkey_pem_end[] asm("_binary_prvtkey_pem_end");
  conf.prvtkey_pem = prvtkey_pem_start;
  conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

  esp_err_t ret = httpd_ssl_start(&server, &conf);
  if (ESP_OK != ret) {
    LOGI(TAG, "Error starting server!");
    return NULL;
  }

  // Set URI handlers
  LOGI(TAG, "Registering URI handlers");
  httpd_register_uri_handler(server, &root);
  httpd_register_uri_handler(server, &uri_post);
  return server;
}

/**
 * @brief https stop webserver
 *
 * @param httpd_req_t HTTP Request Data Structure
 */
static void stop_webserver(httpd_handle_t server) {
  if (server) {
    // Un-register uri handler
    httpd_unregister_uri_handler(server, "/", HTTP_GET);
    httpd_unregister_uri_handler(server, "/", HTTP_POST);
    // Stop the httpd server
    httpd_ssl_stop(server);
  }
}

int ap_connect_handler(void *arg) {
  if (httpd_handle == NULL) {
    LOGI(TAG, "Start Webserver in ap_connect_handler...");
    httpd_handle = start_webserver();
  }
  return 0;
}

int ap_disconnect_handler(void *arg) {
  if (httpd_handle) {
    LOGI(TAG, "Stop Webserver in ap_disconnect_handler...");
    stop_webserver(httpd_handle);
    httpd_handle = NULL;
  }
  return 0;
}

int sta_disconnect_handler(void *arg) {
  if (httpd_handle) {
    LOGI(TAG, "Stop Webserver in sta_disconnect_handler...");
    stop_webserver(httpd_handle);
    httpd_handle = NULL;
  }
  if (s_wifi_event_group) {
    xEventGroupSetBits(s_wifi_event_group, WIFI_DISCONNECT);
  }
  return 0;
}

int sta_ip_handler(void *arg) {
  if (httpd_handle == NULL && farmip[0] == 0) {
    LOGI(TAG, "Start Webserver in sta_ip_handler...");
    httpd_handle = start_webserver();
  }
  if (s_wifi_event_group) {
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED);
  }
  return 0;
}

void easy_setup_task(void *pvParameters) {
  int exit = 0;
  uint8_t mac[6] = { 0 };
  char model_name[10] = { 0 };

  setup_mode_t curr_mode = UNCONFIGURED_MODE;

  syscfg_get(SYSCFG_I_SSID, SYSCFG_N_SSID, farmssid, sizeof(farmssid));
  syscfg_get(SYSCFG_I_PASSWORD, SYSCFG_N_PASSWORD, farmpw, sizeof(farmpw));
  syscfg_get(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip, sizeof(farmip));
  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));

  if (farmssid[0] && farmpw[0]) {
    curr_mode = PAIRING_MODE;
  } else {
    curr_mode = UNCONFIGURED_MODE;
    set_device_configured(0);
  }

  while (1) {
    switch (curr_mode) {
      case UNCONFIGURED_MODE: {
        if (ap_mode_running == 0) {
          esp_read_mac(mac, ESP_MAC_WIFI_STA);
          /* snprintf() is more secure than sprintf() */
          snprintf(AP_SSID, sizeof(AP_SSID), "%s-%02X%02X%02X%02X%02X%02X", model_name, mac[0], mac[1], mac[2], mac[3],
                   mac[4], mac[5]);

          wifi_ap_mode(AP_SSID, WIFI_PASS);

          sysevent_get_with_handler(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, ap_connect_handler, NULL);
          sysevent_get_with_handler(WIFI_EVENT, WIFI_EVENT_AP_STOP, ap_disconnect_handler, NULL);

          ap_mode_running = 1;
        } else {
          syscfg_get(SYSCFG_I_SSID, SYSCFG_N_SSID, farmssid, sizeof(farmssid));
          syscfg_get(SYSCFG_I_PASSWORD, SYSCFG_N_PASSWORD, farmpw, sizeof(farmpw));
          if (farmssid[0] && farmpw[0]) {
            sysevent_unregister_handler(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, ap_connect_handler);
            sysevent_unregister_handler(WIFI_EVENT, WIFI_EVENT_AP_STOP, ap_disconnect_handler);
            curr_mode = PAIRING_MODE;
          }
        }
      } break;
      case PAIRING_MODE: {
        sysevent_get_with_handler(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, sta_disconnect_handler, NULL);
        sysevent_get_with_handler(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_ip_handler, NULL);
        wifi_stop_mode();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        /*** NOTES ***/
        // If the connection is lost, the wifi disconnection information is saved in the nvs area,
        // and due to this, when the device is rebooted, the normal connection may not be possible
        // to refer to the information of the nvs for fast connection.
        // To prevent this issue, nvs_erase() should be called before connection to the AP.
        nvs_erase();
        curr_mode = STA_CONNECT_MODE;
      } break;
      case STA_CONNECT_MODE: {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        wifi_sta_mode();
        // 배터리모델이거나, 이지셋업 중에는 wifi 스캔하지않음 - wifi스캔으로 인한 지연시간이 걸려 셋업에 문제가 생김
        if (!is_battery_model() && farmip[0]) {
          char *best_ssid;
          if ((best_ssid = get_best_wifi_ssid()) && strcmp(farmssid, best_ssid) != 0) {
            snprintf(farmssid, sizeof(farmssid), "%s", best_ssid);
            syscfg_set(SYSCFG_I_SSID, SYSCFG_N_SSID, farmssid);
          }
          if (best_ssid) {
            free(best_ssid);
          }
        }
        wifi_connect_ap(farmssid, farmpw);
        LOGI(TAG, "Connecting to AP with SSID:%s password:%s", farmssid, farmpw);
        curr_mode = CONFIRM_MODE;
      } break;
      case CONFIRM_MODE: {
        // Waiting for event message to check if the connection is success.
        /* Waiting until either the connection is established (WIFI_CONNECTED) or connection failed (WIFI_DISCONNECT)
         * The bits are set by event handler (see above) */
        EventBits_t bits =
            xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED | WIFI_DISCONNECT, pdFALSE, pdFALSE, portMAX_DELAY);
        if (bits & WIFI_CONNECTED) {
          LOGI(TAG, "connected to ap SSID:%s password:%s", farmssid, farmpw);
          curr_mode = PAIRED_MODE;
          xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED);
        } else if (bits & WIFI_DISCONNECT) {
          LOGI(TAG, "Failed to connect to SSID:%s, password:%s", farmssid, farmpw);
          curr_mode = PAIRING_MODE;
          s_retry_connect++;
          xEventGroupClearBits(s_wifi_event_group, WIFI_DISCONNECT);
          vTaskDelay((5000) / portTICK_PERIOD_MS);
          if (s_retry_connect >= MAX_RETRY_CONNECT) {
            set_wifi_fail(1);
            s_retry_connect = 0;
            exit = 1;
          }
        } else {
          LOGE(TAG, "UNEXPECTED EVENT");
          // curr_mode = UNPAIRED_MODE;
          curr_mode = PAIRING_MODE;
        }
      } break;
      case PAIRED_MODE: {
        router_connect = 1;
        if (farmip[0]) {
          LOGI(TAG, "!!! EasySetup is DONE !!!");
          exit = 1;
        } else {
          initialise_mdns();
          curr_mode = SERVER_GET_MODE;
        }
      } break;
      case SERVER_GET_MODE: {
        if (farmip[0]) {
          LOGI(TAG, "!!! EasySetup is DONE !!!");
          ap_disconnect_handler(NULL);
          mdns_free();
          exit = 1;
        }
      } break;
      case UNPAIRED_MODE: {
        LOGI(TAG, "!!! UNPAIRED MODE !!!");
        ap_mode_running = 0;
        wifi_stop_mode();
        syscfg_unset(SYSCFG_I_SSID, SYSCFG_N_SSID);
        syscfg_unset(SYSCFG_I_PASSWORD, SYSCFG_N_PASSWORD);
        curr_mode = UNCONFIGURED_MODE;
        router_connect = 0;
        set_device_configured(0);
      } break;
    }
    if (exit) {
      if (!is_wifi_fail()) {
        set_gateway_address();
        set_device_onboard(1);
        set_device_configured(1);
      }
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  SLOGI(TAG, "Exit EasySetup Task!!!");
  sysevent_unregister_handler(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, sta_disconnect_handler);
  sysevent_unregister_handler(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_ip_handler);
  easy_setup_handle = NULL;

  vEventGroupDelete(s_wifi_event_group);
  s_wifi_event_group = NULL;
  vTaskDelete(NULL);
}

void create_easy_setup_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (easy_setup_handle) {
    LOGI(TAG, "EasySetup task is alreay created");
    return;
  }

  ap_mode_running = 0;
  s_wifi_event_group = xEventGroupCreate();

  xTaskCreate((TaskFunction_t)easy_setup_task, "easy_setup", stack_size, NULL, task_priority, &easy_setup_handle);
}

bool is_running_setup_task(void) {
  return (easy_setup_handle != NULL);
}
