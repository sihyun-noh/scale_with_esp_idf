/**
 * @file sysevent_impl.c
 *
 * @brief system event implementation for esp32 platform
 * Sysevent is designed for the purpose of triggering and receiving events between task applications using event_id.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "sysevent_impl.h"
// #include "monitoring.h"
#include "sys_status.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "esp_wifi_types.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_netif_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "log.h"
#include "sys/queue.h"

/**
 * @brief Define of Sysevent base
 */
ESP_EVENT_DEFINE_BASE(SYSEVENT_BASE);

#define MAX_EVENT_MSG 200

#define NO_EVENT_BASE "NO_EVENT_BASE"
#define NO_EVENT_ID 0

#define SYSEVENT_REQ_QUEUE_SIZE (32)
#define SYSEVENT_RES_QUEUE_SIZE (32)

#define reason2str(r) ((r > 176) ? system_event_reasons[r - 176] : system_event_reasons[r - 1])

const char *system_event_reasons[] = { "UNSPECIFIED",
                                       "AUTH_EXPIRE",
                                       "AUTH_LEAVE",
                                       "ASSOC_EXPIRE",
                                       "ASSOC_TOOMANY",
                                       "NOT_AUTHED",
                                       "NOT_ASSOCED",
                                       "ASSOC_LEAVE",
                                       "ASSOC_NOT_AUTHED",
                                       "DISASSOC_PWRCAP_BAD",
                                       "DISASSOC_SUPCHAN_BAD",
                                       "UNSPECIFIED",
                                       "IE_INVALID",
                                       "MIC_FAILURE",
                                       "4WAY_HANDSHAKE_TIMEOUT",
                                       "GROUP_KEY_UPDATE_TIMEOUT",
                                       "IE_IN_4WAY_DIFFERS",
                                       "GROUP_CIPHER_INVALID",
                                       "PAIRWISE_CIPHER_INVALID",
                                       "AKMP_INVALID",
                                       "UNSUPP_RSN_IE_VERSION",
                                       "INVALID_RSN_IE_CAP",
                                       "802_1X_AUTH_FAILED",
                                       "CIPHER_SUITE_REJECTED",
                                       "BEACON_TIMEOUT",
                                       "NO_AP_FOUND",
                                       "AUTH_FAIL",
                                       "ASSOC_FAIL",
                                       "HANDSHAKE_TIMEOUT",
                                       "CONNECTION_FAIL" };

typedef enum req_cmd { REQ_EVENT = 0, REQ_REGISTER_EVENT_HANDLER, REQ_UNREGISTER_EVENT_HANDLER } req_cmd_t;

portMUX_TYPE msg_count_lock = portMUX_INITIALIZER_UNLOCKED;

typedef struct event_msg {
  char event_base[32];
  uint32_t event_id;
  void *event_data;
  uint32_t event_data_len;
} event_msg_t;

typedef struct event_req {
  req_cmd_t cmd;
  char event_base[32];
  uint32_t event_id;
  event_handler_t event_handler;
  void *handler_data;
} event_req_t;

/**
 * @brief Sysevent event message must be queued until sysevent_get api gets them.
 */
typedef struct event_msg_internal {
  event_msg_t event_msg;
  SLIST_ENTRY(event_msg_internal) next;
} event_msg_internal_t;

/**
 * @brief Request event handler message must wait in the queue until a matching event ID occurs, and then execute the
 * handler when an event occurs.
 */
typedef struct event_handler_internal {
  event_req_t event_req;
  SLIST_ENTRY(event_handler_internal) next;
} event_handler_internal_t;

struct sysevent_ctx {
  QueueHandle_t sysevent_req;
  QueueHandle_t sysevent_res;
  SemaphoreHandle_t sysevent_sem;
  SemaphoreHandle_t sysevent_handler_sem;
  TaskHandle_t sysevent_loop_task;
  TaskHandle_t sysevent_get_task;
  esp_event_handler_instance_t sysevent_handler_instance;
};

/* linked list of event msg for internal use */
static SLIST_HEAD(event_msg_internal_t, event_msg_internal) event_msg_list = SLIST_HEAD_INITIALIZER(event_msg_list);
static SLIST_HEAD(event_handler_internal_t,
                  event_handler_internal) event_handler_list = SLIST_HEAD_INITIALIZER(event_handler_list);

static uint32_t event_msg_cnt = 0;

static const char *TAG = "sysevent";

static event_msg_internal_t *event_msg_find_list(const char *event_base, uint32_t event_id);
static void event_msg_remove_list(event_msg_internal_t *item);
static void event_msg_remove_list_all(void);

static int event_msg_add_list(event_msg_t *event_msg) {
  if (event_msg_cnt >= MAX_EVENT_MSG) {
    LOGI(TAG, "Internal Event messsage list is full");
    event_msg_remove_list_all();
    return -1;
  }

  event_msg_internal_t *item = calloc(1, sizeof(event_msg_internal_t));
  if (item == NULL) {
    LOGE(TAG, "event_msg_add_list: failed to alloc event_msg_internal");
    return -1;
  }

  snprintf(item->event_msg.event_base, sizeof(event_msg->event_base), "%s", event_msg->event_base);
  item->event_msg.event_id = event_msg->event_id;
  item->event_msg.event_data_len = event_msg->event_data_len;
  if (event_msg->event_data_len > 0) {
    item->event_msg.event_data = calloc(1, event_msg->event_data_len);
    if (item->event_msg.event_data == NULL) {
      LOGE(TAG, "event_msg_add_list: failed to alloc event_data");
      free(item);
      return -1;
    }
    memcpy(item->event_msg.event_data, event_msg->event_data, event_msg->event_data_len);
  }

  if (SLIST_EMPTY(&event_msg_list)) {
    SLIST_INSERT_HEAD(&event_msg_list, item, next);
  } else {
    event_msg_internal_t *it = NULL, *last = NULL;
    SLIST_FOREACH(it, &event_msg_list, next) {
      last = it;
    }
    SLIST_INSERT_AFTER(last, item, next);
  }
  portENTER_CRITICAL(&msg_count_lock);
  ++event_msg_cnt;
  portEXIT_CRITICAL(&msg_count_lock);
  return 0;
}

static event_msg_internal_t *event_msg_find_list(const char *event_base, uint32_t event_id) {
  event_msg_internal_t *it = NULL;
  SLIST_FOREACH(it, &event_msg_list, next) {
    if (strcmp(it->event_msg.event_base, event_base) == 0 && it->event_msg.event_id == event_id) {
      return it;
    }
  }
  return NULL;
}

static void event_msg_remove_list(event_msg_internal_t *item) {
  if (item == NULL) {
    return;
  }
  SLIST_REMOVE(&event_msg_list, item, event_msg_internal, next);
  if (item->event_msg.event_data) {
    free(item->event_msg.event_data);
  }
  free(item);
  portENTER_CRITICAL(&msg_count_lock);
  --event_msg_cnt;
  portEXIT_CRITICAL(&msg_count_lock);
}

static void event_msg_remove_list_all(void) {
  event_msg_internal_t *item, *temp;
  SLIST_FOREACH_SAFE(item, &event_msg_list, next, temp) {
    SLIST_REMOVE(&event_msg_list, item, event_msg_internal, next);
    if (item->event_msg.event_data) {
      free(item->event_msg.event_data);
    }
    free(item);
  }
  portENTER_CRITICAL(&msg_count_lock);
  event_msg_cnt = 0;
  portEXIT_CRITICAL(&msg_count_lock);
}

static int event_handler_add_list(event_req_t *event_req) {
  event_handler_internal_t *item = calloc(1, sizeof(event_handler_internal_t));
  if (item == NULL) {
    LOGE(TAG, "event_handler_add_list: failed to alloc event_handler_internal");
    return -1;
  }

  snprintf(item->event_req.event_base, sizeof(event_req->event_base), "%s", event_req->event_base);
  item->event_req.event_id = event_req->event_id;
  if (event_req->event_handler) {
    item->event_req.event_handler = event_req->event_handler;
  }
  if (event_req->handler_data) {
    item->event_req.handler_data = event_req->handler_data;
  }

  if (SLIST_EMPTY(&event_handler_list)) {
    SLIST_INSERT_HEAD(&event_handler_list, item, next);
  } else {
    event_handler_internal_t *it = NULL, *last = NULL;
    SLIST_FOREACH(it, &event_handler_list, next) {
      last = it;
    }
    SLIST_INSERT_AFTER(last, item, next);
  }
  return 0;
}

static event_handler_internal_t *event_handler_find_list(const char *event_base, uint32_t event_id,
                                                         event_handler_t event_handler) {
  event_handler_internal_t *it = NULL;
  SLIST_FOREACH(it, &event_handler_list, next) {
    if (strcmp(it->event_req.event_base, event_base) == 0 && it->event_req.event_id == event_id &&
        it->event_req.event_handler == event_handler) {
      return it;
    }
  }
  return NULL;
}

static void event_handler_remove_list(event_handler_internal_t *item) {
  if (item == NULL) {
    return;
  }
  SLIST_REMOVE(&event_handler_list, item, event_handler_internal, next);
  free(item);
}

static void event_handler_remove_list_all(void) {
  event_handler_internal_t *item, *temp;
  SLIST_FOREACH_SAFE(item, &event_handler_list, next, temp) {
    SLIST_REMOVE(&event_handler_list, item, event_handler_internal, next);
    free(item);
  }
}

static void sysevent_handler(void *handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  LOGI(TAG, "sysevent_handler: event_base=%s, event_id=%d, event_data=%p", event_base, event_id, event_data);

  char ip_addr[32] = { 0 };
  char buf_monitor[50] = { 0 };

  event_msg_t event_msg;
  memset(&event_msg, 0, sizeof(event_msg_t));

  event_msg.event_id = event_id;
  snprintf(event_msg.event_base, sizeof(event_msg.event_base), "%s", event_base);

  if (event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
      snprintf(ip_addr, sizeof(ip_addr), IPSTR, IP2STR(&event->ip_info.ip));
      event_msg.event_data_len = strlen(ip_addr);
      if (event_msg.event_data_len) {
        event_msg.event_data = (char *)calloc(1, event_msg.event_data_len + 1);
        if (event_msg.event_data) {
          memcpy(event_msg.event_data, ip_addr, event_msg.event_data_len);
          LOGI(TAG, "sysevent_handler: event data = %s", (char *)event_msg.event_data);
        }
      }
    } else if (event_base == SYSEVENT_BASE) {
      event_msg.event_data_len = strlen(event_data);
      if (event_msg.event_data_len) {
        event_msg.event_data = (char *)calloc(1, event_msg.event_data_len + 1);
        if (event_msg.event_data) {
          memcpy(event_msg.event_data, event_data, event_msg.event_data_len);
          LOGI(TAG, "sysevent_handler: event data = %s", (char *)event_msg.event_data);
        }
      }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
      wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
      snprintf(buf_monitor, sizeof(buf_monitor), "%s", event->ssid);
      event_msg.event_data_len = strlen(buf_monitor);
      if (event_msg.event_data_len) {
        event_msg.event_data = (char *)calloc(1, event_msg.event_data_len + 1);
        if (event_msg.event_data) {
          memcpy(event_msg.event_data, buf_monitor, event_msg.event_data_len);
          LOGI(TAG, "sysevent_handler: event data = %s", buf_monitor);
        }
      }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
      set_device_onboard(0);
      wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
      uint8_t reason = event->reason;
      snprintf(buf_monitor, sizeof(buf_monitor), "%u - %s", reason, reason2str(reason));
      event_msg.event_data_len = strlen(buf_monitor);
      if (event_msg.event_data_len) {
        event_msg.event_data = (char *)calloc(1, event_msg.event_data_len + 1);
        if (event_msg.event_data) {
          memcpy(event_msg.event_data, buf_monitor, event_msg.event_data_len);
          LOGI(TAG, "sysevent_handler: event data = %s", buf_monitor);
        }
      }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
      set_wifi_scan_done(1);
      set_wifi_scanning(0);
      LOGI(TAG, "sysevent_handler : WIFI_SCAN_DONE!!!");
    }
  }

  // Add the event message received from system event handler to the internal event list
  event_msg_add_list(&event_msg);
  if (event_msg.event_data) {
    free(event_msg.event_data);
  }
}

static int sysevent_loop_run(sysevent_ctx_t *ctx, TickType_t ticks_to_run) {
  xSemaphoreTake(ctx->sysevent_handler_sem, portMAX_DELAY);

  if (!SLIST_EMPTY(&event_handler_list)) {
    event_handler_internal_t *req, *temp_req;

    SLIST_FOREACH_SAFE(req, &event_handler_list, next, temp_req) {
      event_msg_internal_t *item = event_msg_find_list(req->event_req.event_base, req->event_req.event_id);
      if (item) {
        if (req->event_req.event_handler) {
          req->event_req.event_handler(req->event_req.handler_data);
        }
        // Remove the event message from the internal event message list
        event_msg_remove_list(item);
        // Remove the event handler from the internal event handler list
        SLIST_REMOVE(&event_handler_list, req, event_handler_internal, next);
      }
    }
  }

  xSemaphoreGive(ctx->sysevent_handler_sem);

  vTaskDelay(ticks_to_run);
  return 0;
}

static int sysevent_get_request(sysevent_ctx_t *ctx, TickType_t ticks_to_run) {
  event_msg_t event_msg;
  event_req_t event_req;

  if (xQueueReceive(ctx->sysevent_req, &event_req, ticks_to_run) == pdTRUE) {
    xSemaphoreTake(ctx->sysevent_handler_sem, portMAX_DELAY);
    memset(&event_msg, 0, sizeof(event_msg_t));
    event_msg_internal_t *item = event_msg_find_list(event_req.event_base, event_req.event_id);
    if (item) {
      if (event_req.event_handler == NULL && event_req.handler_data == NULL) {
        memset(&event_msg, 0, sizeof(event_msg_t));
        event_msg.event_id = item->event_msg.event_id;
        snprintf(event_msg.event_base, sizeof(event_msg.event_base), "%s", item->event_msg.event_base);
        int data_len = item->event_msg.event_data_len;
        if (data_len) {
          event_msg.event_data = calloc(1, data_len);
          event_msg.event_data_len = data_len;
          if (item->event_msg.event_data) {
            memcpy(event_msg.event_data, item->event_msg.event_data, data_len);
          }
        }
        if (xQueueSendToBack(ctx->sysevent_res, &event_msg, 0) != pdTRUE) {
          if (event_msg.event_data) {
            free(event_msg.event_data);
          }
        }
      } else if (event_req.event_handler) {
        LOGI(TAG, "Call event_handler for %s and %d", event_req.event_base, event_req.event_id);
        event_req.event_handler(event_req.handler_data);
      }
      // Remove the event message from the internal event message list
      event_msg_remove_list(item);
    } else {
      if (event_req.event_handler && event_req.cmd == REQ_REGISTER_EVENT_HANDLER) {
        // If event_handler needs to be called, add it to the event_handler list and call it later when the event
        // occurs.
        LOGI(TAG, "Add %s and %d to event_handler list for later calls", event_req.event_base, event_req.event_id);
        event_handler_add_list(&event_req);
      } else if (event_req.event_handler && event_req.cmd == REQ_UNREGISTER_EVENT_HANDLER) {
        event_handler_internal_t *handler_item =
            event_handler_find_list(event_req.event_base, event_req.event_id, event_req.event_handler);
        if (handler_item) {
          event_handler_remove_list(handler_item);
        }
      } else {
        memset(&event_msg, 0, sizeof(event_msg_t));
        snprintf(event_msg.event_base, sizeof(event_msg.event_base), "%s", NO_EVENT_BASE);
        event_msg.event_id = NO_EVENT_ID;
        xQueueSendToBack(ctx->sysevent_res, &event_msg, 0);
      }
    }

    memset(&event_req, 0, sizeof(event_req_t));
    xSemaphoreGive(ctx->sysevent_handler_sem);
  }

  return 0;
}

void sysevent_loop_run_task(void *arg) {
  int ret = 0;
  sysevent_ctx_t *ctx = (sysevent_ctx_t *)arg;

  while (1) {
    ret = sysevent_loop_run(ctx, 1000 / portTICK_PERIOD_MS);
    if (ret != 0) {
      break;
    }
  }
  vTaskSuspend(NULL);
}

void sysevent_get_task(void *arg) {
  int ret = 0;
  sysevent_ctx_t *ctx = (sysevent_ctx_t *)arg;

  while (1) {
    ret = sysevent_get_request(ctx, portMAX_DELAY);
    if (ret != 0) {
      break;
    }
  }
  vTaskSuspend(NULL);
}

sysevent_ctx_t *sysevent_create_impl(void) {
  // Singletone sysevent context
  static sysevent_ctx_t *ctx = NULL;

  if (ctx == NULL) {
    ctx = (sysevent_ctx_t *)calloc(1, sizeof(sysevent_ctx_t));
    if (ctx == NULL) {
      LOGE(TAG, "Failed to allocate memory for sysevent context");
      goto err_ctx;
    }

    ctx->sysevent_req = xQueueCreate(SYSEVENT_REQ_QUEUE_SIZE, sizeof(event_req_t));
    if (ctx->sysevent_req == NULL) {
      goto err_sysevent_req;
    }
    ctx->sysevent_res = xQueueCreate(SYSEVENT_RES_QUEUE_SIZE, sizeof(event_msg_t));
    if (ctx->sysevent_res == NULL) {
      goto err_sysevent_res;
    }
    ctx->sysevent_sem = xSemaphoreCreateMutex();
    if (ctx->sysevent_sem == NULL) {
      LOGE(TAG, "Failed to create sysevent message semaphore");
      goto err_sysevent_sem;
    }
    ctx->sysevent_handler_sem = xSemaphoreCreateMutex();
    if (ctx->sysevent_handler_sem == NULL) {
      LOGE(TAG, "Failed to create sysevent handler semaphore");
      goto err_sysevent_handler_sem;
    }

    esp_err_t ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
      LOGE(TAG, "Failed to create esp_event_loop_handler");
      goto err_loop_create;
    }

    ret = esp_event_handler_instance_register(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, &sysevent_handler, NULL,
                                              &ctx->sysevent_handler_instance);
    if (ret != ESP_OK) {
      LOGE(TAG, "Failed to register sysevent handler");
      goto err_handler_reg;
    }

    if (xTaskCreate(&sysevent_loop_run_task, "sysevent_loop_run_task", 4096, ctx, 5, &ctx->sysevent_loop_task) !=
        pdTRUE) {
      LOGE(TAG, "Failed to create sysevent loop task");
      goto err_loop_task_create;
    }

    if (xTaskCreate(&sysevent_get_task, "sysevent_get_task", 4096, ctx, 5, &ctx->sysevent_get_task) != pdTRUE) {
      LOGE(TAG, "Failed to create sysevent get task");
      goto err_get_task_create;
    }

    LOGI(TAG, "Sysevent context created");
  }
  return ctx;

  /* Error Handling */
err_get_task_create:
  vTaskDelete(ctx->sysevent_loop_task);
err_loop_task_create:
  esp_event_handler_instance_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, ctx->sysevent_handler_instance);
err_handler_reg:
  esp_event_loop_delete_default();
err_loop_create:
  vSemaphoreDelete(ctx->sysevent_handler_sem);
err_sysevent_handler_sem:
  vSemaphoreDelete(ctx->sysevent_sem);
err_sysevent_sem:
  vQueueDelete(ctx->sysevent_res);
err_sysevent_res:
  vQueueDelete(ctx->sysevent_req);
err_sysevent_req:
err_ctx:
  free(ctx);
  ctx = NULL;
  return NULL;
}

void sysevent_destroy_impl(sysevent_ctx_t *ctx) {
  if (ctx == NULL) {
    return;
  }

  event_msg_remove_list_all();
  event_handler_remove_list_all();

  esp_event_handler_instance_unregister(ESP_EVENT_ANY_BASE, ESP_EVENT_ANY_ID, ctx->sysevent_handler_instance);
  esp_event_loop_delete_default();

  if (ctx->sysevent_req) {
    vQueueDelete(ctx->sysevent_req);
  }
  if (ctx->sysevent_res) {
    vQueueDelete(ctx->sysevent_res);
  }
  if (ctx->sysevent_sem) {
    vSemaphoreDelete(ctx->sysevent_sem);
  }
  if (ctx->sysevent_handler_sem) {
    vSemaphoreDelete(ctx->sysevent_handler_sem);
  }

  if (ctx->sysevent_loop_task) {
    vTaskDelete(ctx->sysevent_loop_task);
  }
  if (ctx->sysevent_get_task) {
    vTaskDelete(ctx->sysevent_get_task);
  }
  free(ctx);
}

int sysevent_set_impl(sysevent_ctx_t *ctx, int event_id, void *event_data) {
  if (ctx == NULL || event_data == NULL) {
    return -1;
  }

  xSemaphoreTake(ctx->sysevent_sem, portMAX_DELAY);

  int len = strlen((char *)event_data);
  esp_err_t ret = esp_event_post(SYSEVENT_BASE, event_id, (char *)event_data, len + 1, 100 / portTICK_PERIOD_MS);

  xSemaphoreGive(ctx->sysevent_sem);

  return (ret == ESP_OK) ? 0 : -1;
}

int sysevent_get_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id, void *event_data,
                      size_t event_data_len) {
  int ret = -1;
  if (ctx == NULL || event_base == NULL) {
    return ret;
  }

  event_msg_t event_msg;
  event_req_t event_req;
  memset(&event_msg, 0, sizeof(event_msg_t));
  memset(&event_req, 0, sizeof(event_req_t));

  xSemaphoreTake(ctx->sysevent_sem, portMAX_DELAY);

  snprintf(event_req.event_base, sizeof(event_req.event_base), "%s", event_base);
  event_req.cmd = REQ_EVENT;
  event_req.event_id = event_id;

  if (xQueueSendToBack(ctx->sysevent_req, &event_req, 0) != pdPASS) {
    goto _exit;
  }

  // LOGI(TAG, "Wait for event_base = %s, event_id = %d", event_base, event_id);

  if (xQueueReceive(ctx->sysevent_res, &event_msg, portMAX_DELAY) == pdTRUE) {
    //   LOGI(TAG, "Got event_base = %s, event_id = %d", event_msg.event_base, event_msg.event_id);
    if (strcmp(event_msg.event_base, NO_EVENT_BASE) == 0 && event_msg.event_id == NO_EVENT_ID) {
      // No event in accordance with event base and event id
      goto _exit;
    }
    if (strcmp(event_req.event_base, event_msg.event_base) == 0 && (event_req.event_id == event_msg.event_id)) {
      if (event_msg.event_data && event_msg.event_data_len) {
        int data_len = (event_data_len > event_msg.event_data_len) ? event_msg.event_data_len : event_data_len;
        memcpy(event_data, event_msg.event_data, data_len);
      }
      ret = 0;
    }
    if (event_msg.event_data) {
      free(event_msg.event_data);
    }
  }

_exit:
  xSemaphoreGive(ctx->sysevent_sem);
  return ret;
}

int sysevent_get_with_handler_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id,
                                   event_handler_t event_handler, void *handler_data) {
  int ret = -1;
  if (ctx == NULL || event_base == NULL || event_handler == NULL) {
    LOGI(TAG, "sysevent_get_with_handler_impl : Invalid parameter");
    return ret;
  }

  xSemaphoreTake(ctx->sysevent_sem, portMAX_DELAY);

  event_req_t event_req;
  memset(&event_req, 0, sizeof(event_req_t));

  snprintf(event_req.event_base, sizeof(event_req.event_base), "%s", event_base);
  event_req.cmd = REQ_REGISTER_EVENT_HANDLER;
  event_req.event_id = event_id;
  event_req.event_handler = event_handler;
  event_req.handler_data = handler_data;

  if (xQueueSendToBack(ctx->sysevent_req, &event_req, 0) == pdPASS) {
    ret = 0;
  }

  xSemaphoreGive(ctx->sysevent_sem);
  return ret;
}

int sysevent_unregister_handler_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id,
                                     event_handler_t event_handler) {
  int ret = -1;
  if (ctx == NULL || event_base == NULL || event_handler == NULL) {
    LOGI(TAG, "sysevent_unregister_handler_impl : Invalid parameter");
    return ret;
  }

  xSemaphoreTake(ctx->sysevent_sem, portMAX_DELAY);

  event_req_t event_req;
  memset(&event_req, 0, sizeof(event_req_t));

  snprintf(event_req.event_base, sizeof(event_req.event_base), "%s", event_base);
  event_req.cmd = REQ_UNREGISTER_EVENT_HANDLER;
  event_req.event_id = event_id;
  event_req.event_handler = event_handler;
  event_req.handler_data = NULL;

  if (xQueueSendToBack(ctx->sysevent_req, &event_req, 0) == pdPASS) {
    ret = 0;
  }

  xSemaphoreGive(ctx->sysevent_sem);
  return ret;
}
