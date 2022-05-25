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
#include <esp_https_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <sys/param.h>

#include "syscfg.h"
#include "sysevent.h"
#include "syslog.h"
#include "wifi_manager.h"

//#include "time_api.h"
#include "cJSON.h"
#include "mdns.h"

#define MODEL_NAME "GLSTH"
#define WIFI_PASS "!ALfE42vcchYpFQyPuCN*v_w"

/* The event group allows multiple bits for each event, but we only care about
 * two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED BIT0
#define WIFI_DISCONNECT BIT1

#define MAX_RETRY_CONNECT 5

typedef enum {
  UNCONFIGURED_MODE = 0,
  PAIRING_MODE,
  CONFIRM_MODE,
  PAIRED_MODE,
  UNPAIRED_MODE,
  SERVER_GET_MODE
} setup_mode_t;

static const char *TAG = "easy_setup";

static TaskHandle_t easy_setup_handle = NULL;
static httpd_handle_t httpd_hanlde = NULL;

static int ap_mode_running = 0;
static int s_retry_connect = 0;

static int router_connect = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

extern void initialise_mdns(void);
extern void create_http_test_task(void);

char AP_SSID[30] = { 0 };
char farmssid[50] = { 0 };
char farmpw[50] = { 0 };
char farmip[80] = { 0 };

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
    // ssid pw 검색
    snprintf(farmssid, sizeof(farmssid), "%s", ssid->valuestring);
    snprintf(farmpw, sizeof(farmpw), "%s", pw->valuestring);
    syscfg_set(CFG_DATA, "ssid", farmssid);
    syscfg_set(CFG_DATA, "password", farmpw);
    LOGI(TAG, "Got SSID = [%s] password = [%s]", farmssid, farmpw);
  }

  cJSON *Server = cJSON_GetObjectItem(root, "Server");
  if (cJSON_IsString(Server)) {
    // "http://223.171.52.138:8803/api/v1/wifi-sensor"
    snprintf(farmip, sizeof(farmip), "http://%s/api/v1/wifi-sensor", Server->valuestring);
    syscfg_set(CFG_DATA, "farmip", farmip);
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

  conf.port_secure = 30001;  // 30001 Port 로 전송

  extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
  extern const unsigned char cacert_pem_end[] asm("_binary_cacert_pem_end");
  conf.cacert_pem = cacert_pem_start;
  conf.cacert_len = cacert_pem_end - cacert_pem_start;

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
  if (httpd_hanlde == NULL) {
    httpd_hanlde = start_webserver();
  }
  return 0;
}

int ap_disconnect_handler(void *arg) {
  if (httpd_hanlde) {
    stop_webserver(httpd_hanlde);
    httpd_hanlde = NULL;
  }
  return 0;
}

int sta_disconnect_handler(void *arg) {
  if (httpd_hanlde) {
    stop_webserver(httpd_hanlde);
    httpd_hanlde = NULL;
  }
  xEventGroupSetBits(s_wifi_event_group, WIFI_DISCONNECT);
  return 0;
}

int sta_ip_handler(void *arg) {
  if (httpd_hanlde == NULL) {
    httpd_hanlde = start_webserver();
  }
  xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED);
  return 0;
}

void easy_setup_task(void *pvParameters) {
  int exit = 0;
  uint8_t mac[6] = { 0 };
  setup_mode_t curr_mode = UNCONFIGURED_MODE;

  syscfg_get(CFG_DATA, "ssid", farmssid, sizeof(farmssid));
  syscfg_get(CFG_DATA, "password", farmpw, sizeof(farmpw));
  syscfg_get(CFG_DATA, "farmip", farmip, sizeof(farmip));

  if (farmssid[0] && farmpw[0]) {
    curr_mode = PAIRING_MODE;
  } else {
    curr_mode = UNCONFIGURED_MODE;
  }

  while (1) {
    switch (curr_mode) {
      case UNCONFIGURED_MODE: {
        if (ap_mode_running == 0) {
          esp_read_mac(mac, ESP_MAC_WIFI_STA);
          /* snprintf() is more secure than sprintf() */
          snprintf(AP_SSID, sizeof(AP_SSID), "%s-%02X%02X%02X%02X%02X%02X", MODEL_NAME, mac[0], mac[1], mac[2], mac[3],
                   mac[4], mac[5]);

          wifi_ap_mode(AP_SSID, WIFI_PASS);

          sysevent_get_with_handler(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, ap_connect_handler, NULL);
          sysevent_get_with_handler(WIFI_EVENT, WIFI_EVENT_AP_STOP, ap_disconnect_handler, NULL);

          ap_mode_running = 1;
        } else {
          syscfg_get(CFG_DATA, "ssid", farmssid, sizeof(farmssid));
          syscfg_get(CFG_DATA, "password", farmpw, sizeof(farmpw));
          if (farmssid[0] && farmpw[0]) {
            curr_mode = PAIRING_MODE;
          }
        }
      } break;
      case PAIRING_MODE: {
        sysevent_get_with_handler(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, sta_disconnect_handler, NULL);
        sysevent_get_with_handler(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_ip_handler, NULL);
        wifi_stop_mode();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        wifi_sta_mode();
        wifi_connect_ap(farmssid, farmpw);
        LOGI(TAG, "Connecting to AP with SSID:%s password:%s", farmssid, farmpw);
        curr_mode = CONFIRM_MODE;
      } break;
      case CONFIRM_MODE: {
        // Waiting for event message to check if the connection is success.
        /* Waiting until either the connection is established (WIFI_CONNECTED)
         * or connection failed (WIFI_DISCONNECT) The bits are set by event
         * handler (see above) */
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
          vTaskDelay(500 / portTICK_PERIOD_MS);
          if (s_retry_connect >= MAX_RETRY_CONNECT) {
            curr_mode = UNPAIRED_MODE;
            s_retry_connect = 0;
          }
        } else {
          LOGE(TAG, "UNEXPECTED EVENT");
          curr_mode = UNPAIRED_MODE;
        }
      } break;
      case PAIRED_MODE: {
        initialise_mdns();
        router_connect = 1;
        /* if (tm_is_timezone_set() == false) { */
        /*   tm_init_sntp(); */
        /*   tm_apply_timesync(); */
        /*   // TODO : Get the timezone using the region code of the
         * manufacturing data. */
        /*   // Currently hardcoding the timezone to be KST-9 */
        /*   tm_set_timezone("Asia/Seoul"); */
        /* } */
        curr_mode = SERVER_GET_MODE;
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
        syscfg_unset(CFG_DATA, "ssid");
        syscfg_unset(CFG_DATA, "password");
        curr_mode = UNCONFIGURED_MODE;
        router_connect = 0;
      } break;
    }
    if (exit) {
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  SLOGI(TAG, "Exit EasySetup Task!!!");
  easy_setup_handle = NULL;
  vEventGroupDelete(s_wifi_event_group);
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

int is_router_connect(void) {
  return router_connect;
}
