#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_sleep.h"

#include <string.h>
#include <time.h>

#include "ui.h"
#include "ui_helpers.h"
#include "log.h"
#include "espnow.h"
#include "time_api.h"
#include "sys_status.h"
#include "comm_packet.h"
#include "hid_config.h"
#include "command.h"
#include "sysfile.h"
#include "filelog.h"
#include "gui.h"

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

static bool _send_msg_event(irrigation_message_t *msg) {
  return ctrl_msg_queue && (xQueueSend(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static bool _get_msg_event(irrigation_message_t *msg) {
  return ctrl_msg_queue && (xQueueReceive(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

void send_msg_to_ctrl_task(void *msg, size_t msg_len) {
  irrigation_message_t message = { 0 };

  if (msg && (msg_len == sizeof(irrigation_message_t))) {
    memcpy(&message, msg, msg_len);
    if (!_send_msg_event(&message)) {
      LOGI(TAG, "Failed to send ctrl messag event!!!");
    }
  }
}

void set_main_time(time_t *curr_time) {
  struct tm timeinfo = { 0 };
  localtime_r(curr_time, &timeinfo);
  set_local_time(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                 timeinfo.tm_sec);
  // set time sync event flag
  set_time_sync(1);
  LOGI(TAG, "main time is synced");
  FDATA(BASE_PATH, "%s", "Time synchronization is complete");
}

void ctrl_msg_handler(irrigation_message_t *message) {
  switch (message->sender_type) {
    case TIME_SYNC: {
      set_main_time(&message->current_time);
      lv_msg_send(MSG_TIME_SYNCED, NULL);
    } break;
    case RESPONSE: {
      if (message->receive_type == SET_CONFIG && message->resp == SUCCESS) {
        LOGI(TAG, "Success SET_CONFIG, disable start button");
        lv_msg_send(MSG_RESPONSE_STATUS, message);
      } else if (message->receive_type == FORCE_STOP && message->resp == SUCCESS) {
        LOGI(TAG, "Success FORCE_STOP, enable start button");
        lv_msg_send(MSG_RESPONSE_STATUS, message);
      }
    } break;
    case BATTERY_LEVEL: {
      send_command_data(RESPONSE, message->sender_type, NULL, 0);
      // add battery level to the logging data
      device_status_t *dev_stat = (device_status_t *)&message->payload.dev_stat;
      for (int id = 1; id < 7; id++) {
        LOGI(TAG, "Zone[%d] : Battery level = %d", id, dev_stat->battery_level[id]);
      }
      set_battery_level(1);
      lv_msg_send(MSG_BATTERY_STATUS, message);
      if (!is_time_sync()) {
        set_main_time(&message->current_time);
        LOGI(TAG, "Call timesync in Battery level");
        lv_msg_send(MSG_TIME_SYNCED, NULL);
      }
    } break;
    case START_FLOW: {
      send_command_data(RESPONSE, message->sender_type, NULL, 0);
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
    } break;
    case ZONE_COMPLETE: {
      send_command_data(RESPONSE, message->sender_type, NULL, 0);
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
    } break;
    case ALL_COMPLETE: {
      send_command_data(RESPONSE, message->sender_type, NULL, 0);
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
    } break;
    case SET_SLEEP: {
      uint64_t wakeup_time_sec = message->payload.remain_time_sleep;
      FDATA(BASE_PATH, "Entering sleep mode with time = %llus", wakeup_time_sec);
      LOGI(TAG, "Entering sleep mode wtih time = %llus", wakeup_time_sec);
      esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
      esp_deep_sleep_start();
    } break;
    case DEVICE_ERROR: {
      send_command_data(RESPONSE, message->sender_type, NULL, 0);
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
    } break;
    default: break;
  }
}

static void control_task(void *pvParameter) {
  irrigation_message_t msg;

  for (;;) {
    memset(&msg, 0x00, sizeof(irrigation_message_t));
    if (_get_msg_event(&msg)) {
      ctrl_msg_handler(&msg);
    } else {
      LOGE(TAG, "Failed to receive ctrl message form queue");
      vTaskDelay(1000);
      continue;
    }
    vTaskDelay(500);
  }
  vTaskDelete(NULL);
  control_handle = NULL;
}

void create_control_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY;

  if (control_handle) {
    LOGI(TAG, "control task is already running...");
    return;
  }

  // create queue
  ctrl_msg_queue = xQueueCreate(CTRL_MSG_QUEUE_LEN, sizeof(irrigation_message_t));
  if (ctrl_msg_queue == NULL) {
    return;
  }

  xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 0);
}
