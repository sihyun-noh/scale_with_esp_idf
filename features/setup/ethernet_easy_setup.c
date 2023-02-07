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
#include "sys_status.h"
#include "event_ids.h"
#include "syslog.h"
#include "wifi_manager.h"

#include <sys/param.h>
#include <freertos/FreeRTOS.h>
#include "freertos/event_groups.h"
#include <esp_https_server.h>

#include "cJSON.h"
#include "mdns.h"
#include "ethernet_manager.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define ETHERNET_CONNECTED BIT0
#define ETHERNET_DISCONNECT BIT1
#define MAX_RETRY_CONNECT 5

typedef enum {
  UNCONFIGURED_MODE = 0,
  PAIRED_MODE,
  UNPAIRED_MODE,
  SERVER_GET_MODE,
  ETHERNET_SET_MODE,
  ETHERNET_CONFIRM_MODE
} setup_mode_t;

static const char *TAG = "easy_setup";

static TaskHandle_t easy_setup_handle = NULL;
static httpd_handle_t httpd_handle = NULL;

static int s_retry_connect = 0;
static int router_connect = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_ethernet_event_group;

extern void initialise_mdns(void);

char farmip[30] = { 0 };

static void set_gateway_address(void) {
  char router_ip[20] = { 0 };

  get_router_ipaddr(router_ip, sizeof(router_ip));
  if (router_ip[0]) {
    syscfg_set(CFG_DATA, "gateway", router_ip);
  }
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

  cJSON *Server = cJSON_GetObjectItem(root, "Server");
  if (cJSON_IsString(Server)) {
    snprintf(farmip, sizeof(farmip), "%s", Server->valuestring);
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

int sta_disconnect_handler(void *arg) {
  if (httpd_handle) {
    LOGI(TAG, "Stop Webserver in sta_disconnect_handler...");
    stop_webserver(httpd_handle);
    httpd_handle = NULL;
  }
  if (s_ethernet_event_group) {
    xEventGroupSetBits(s_ethernet_event_group, ETHERNET_DISCONNECT);
  }
  return 0;
}

int sta_ip_handler(void *arg) {
  if (httpd_handle == NULL && farmip[0] == 0) {
    LOGI(TAG, "Start Webserver in sta_ip_handler...");
    httpd_handle = start_webserver();
  }
  if (s_ethernet_event_group) {
    xEventGroupSetBits(s_ethernet_event_group, ETHERNET_CONNECTED);
  }
  return 0;
}

void ethernet_easy_setup_task(void *pvParameters) {
  int exit = 0;
  setup_mode_t curr_mode = UNCONFIGURED_MODE;

  set_device_configured(0);

  while (1) {
    switch (curr_mode) {
      case UNCONFIGURED_MODE: {
        syscfg_get(CFG_DATA, "farmip", farmip, sizeof(farmip));
        curr_mode = ETHERNET_SET_MODE;
      } break;
      case ETHERNET_SET_MODE: {
        sysevent_get_with_handler(IP_EVENT, IP_EVENT_ETH_LOST_IP, sta_disconnect_handler, NULL);
        sysevent_get_with_handler(IP_EVENT, IP_EVENT_ETH_GOT_IP, sta_ip_handler, NULL);
        ethernet_init();
        LOGI(TAG, "Connecting to Ethernet");
        curr_mode = ETHERNET_CONFIRM_MODE;
      } break;
      case ETHERNET_CONFIRM_MODE: {
        // Waiting for event message to check if the connection is success.
        /* Waiting until either the connection is established (ETHERNET_CONNECTED) or connection failed
         * (ETHERNET_DISCONNECT) The bits are set by event handler (see above) */
        EventBits_t bits = xEventGroupWaitBits(s_ethernet_event_group, ETHERNET_CONNECTED | ETHERNET_DISCONNECT,
                                               pdFALSE, pdFALSE, portMAX_DELAY);
        if (bits & ETHERNET_CONNECTED) {
          LOGI(TAG, "connected to Ethernet");
          curr_mode = PAIRED_MODE;
        } else if (bits & ETHERNET_DISCONNECT) {
          LOGI(TAG, "Failed to connect to Ethernet");
          curr_mode = ETHERNET_SET_MODE;
          s_retry_connect++;
          xEventGroupClearBits(s_ethernet_event_group, ETHERNET_DISCONNECT);
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
          sta_disconnect_handler(NULL);
          mdns_free();
          exit = 1;
        }
      } break;
      case UNPAIRED_MODE: {
        LOGI(TAG, "!!! UNPAIRED MODE !!!");
        curr_mode = UNCONFIGURED_MODE;
        router_connect = 0;
        set_device_configured(0);
      } break;
    }
    if (exit) {
      set_gateway_address();
      set_device_onboard(1);
      set_device_configured(1);
      break;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  SLOGI(TAG, "Exit EasySetup Task!!!");
  sysevent_unregister_handler(IP_EVENT, IP_EVENT_ETH_LOST_IP, sta_disconnect_handler);
  sysevent_unregister_handler(IP_EVENT, IP_EVENT_STA_GOT_IP, sta_ip_handler);
  easy_setup_handle = NULL;

  vEventGroupDelete(s_ethernet_event_group);
  vTaskDelete(NULL);
}

void create_ethernet_easy_setup_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (easy_setup_handle) {
    LOGI(TAG, "EasySetup task is alreay created");
    return;
  }

  s_ethernet_event_group = xEventGroupCreate();

  xTaskCreate((TaskFunction_t)ethernet_easy_setup_task, "easy_setup", stack_size, NULL, task_priority,
              &easy_setup_handle);
}

int is_router_connect(void) {
  return router_connect;
}
