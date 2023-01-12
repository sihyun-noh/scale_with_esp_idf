#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <string.h>
#include <time.h>

#include "ui_helpers.h"
#include "log.h"
#include "espnow.h"
#include "time_api.h"
#include "sys_status.h"
#include "comm_packet.h"

#define CTRL_MSG_QUEUE_LEN 16

static const char *TAG = "ctrl_task";

static TaskHandle_t control_handle = NULL;
static QueueHandle_t ctrl_msg_queue = NULL;

/***************************************************/
// (1) set_config => resp
// (2) time_sync => time_t : current_time
// (3) start_flow => no data
// (4) zone_complete => flow_value, deviceid
// (5) all_complete => no data
// (6) force stop => resp, flow_value, device_id
// (7) set_sleep => remain_time_sleep
/***************************************************/

static bool _send_msg_event(irrigation_message_t **msg) {
  return ctrl_msg_queue && (xQueueSend(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static bool _get_msg_event(irrigation_message_t **msg) {
  return ctrl_msg_queue && (xQueueReceive(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

void send_msg_to_ctrl_task(void *msg, size_t msg_len) {
  irrigation_message_t *message = NULL;
  if (msg && (msg_len == sizeof(irrigation_message_t))) {
    message = calloc(1, sizeof(irrigation_message_t));
    if (message) {
      memcpy(message, msg, sizeof(irrigation_message_t));
      if (!_send_msg_event(&message)) {
        free((void *)(message));
      }
    }
  }
}

void set_main_time(time_t *curr_time) {
  struct tm timeinfo = { 0 };
  localtime_r(curr_time, &timeinfo);
  set_local_time(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                 timeinfo.tm_sec);
  LOGI(TAG, "main time is synced");
}

bool send_response_data(message_type_t sender) {
  irrigation_message_t resp_data;

  memset(&resp_data, 0x00, sizeof(resp_data));
  resp_data.sender_type = RESPONSE;
  resp_data.receive_type = sender;
  resp_data.resp = SUCCESS;
  resp_data.current_time = get_current_time();

  return espnow_send_data(espnow_get_master_addr(), (uint8_t *)&resp_data, sizeof(resp_data));
}

void ctrl_msg_handler(irrigation_message_t *message) {
  switch (message->sender_type) {
    case SET_CONFIG: break;
    case TIME_SYNC: {
      set_main_time(&message->current_time);
      set_time_sync(1);
      enable_buttons();
    } break;
    case RESPONSE: break;
    case BATTERY_LEVEL: {
      // add battery level to the logging data
      set_battery_level(1);
      send_response_data(message->sender_type);
    } break;
    case START_FLOW: {
      int zone_id = message->deviceId;
      set_zone_status(zone_id, true);
      send_response_data(message->sender_type);
    } break;
    case ZONE_COMPLETE: {
      int zone_id = message->deviceId;
      int flow_value = message->flow_value;
      set_zone_status(zone_id, false);
      set_zone_flow_value(zone_id, flow_value);
      send_response_data(message->sender_type);
    } break;
    case ALL_COMPLETE: {
      enable_start_button();
      send_response_data(message->sender_type);
    } break;
    case SET_SLEEP: break;
    default: break;
  }

  free((void *)message);
}

static void control_task(void *pvParameter) {
  irrigation_message_t *msg = NULL;

  for (;;) {
    if (_get_msg_event(&msg)) {
      ctrl_msg_handler(msg);
    } else {
      LOGE(TAG, "Cannot receive ctrl message form queue");
      continue;
    }
  }
  vTaskDelete(NULL);
  control_handle = NULL;
}

void create_control_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (control_handle) {
    LOGI(TAG, "control task is already running...");
    return;
  }

  ctrl_msg_queue = xQueueCreate(CTRL_MSG_QUEUE_LEN, sizeof(irrigation_message_t));
  if (ctrl_msg_queue == NULL) {
    return;
  }

  xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 1);
}
