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
    memcpy(&message, msg, sizeof(irrigation_message_t));
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
}

void ctrl_msg_handler(irrigation_message_t *message) {
  switch (message->sender_type) {
    case TIME_SYNC: {
      set_main_time(&message->current_time);
      lv_msg_send(MSG_TIME_SYNCED, NULL);
#if 0
      enable_buttons();
#endif
    } break;
    case RESPONSE: {
      if (message->receive_type == SET_CONFIG && message->resp == SUCCESS) {
        LOGI(TAG, "Success SET_CONFIG, disable start button");
        lv_msg_send(MSG_RESPONSE_STATUS, message);
#if 0
        disable_start_button();
#endif
      } else if (message->receive_type == FORCE_STOP && message->resp == SUCCESS) {
        LOGI(TAG, "Success FORCE_STOP, enable start button");
        lv_msg_send(MSG_RESPONSE_STATUS, message);
#if 0
        char op_msg[128] = { 0 };
        int zone_id = message->deviceId;
        int flow_value = message->flow_value;
        if (zone_id >= 1 && zone_id <= 6) {
          set_zone_status(zone_id, false);
          set_zone_number(zone_id, false);
          set_zone_flow_value(zone_id, flow_value);
          enable_start_button();
          snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
          add_operation_list(op_msg);
      }
#endif
      }
    } break;
    case BATTERY_LEVEL: {
      send_command_data(RESPONSE_COMMAND, &message->sender_type, sizeof(message->sender_type));
      // add battery level to the logging data
      for (int id = 1; id < 7; id++) {
        LOGI(TAG, "Zone[%d] : Battery level = %d", id, message->battery_level[id]);
      }
      set_battery_level(1);
      lv_msg_send(MSG_BATTERY_STATUS, message);
      if (!is_time_sync()) {
        set_main_time(&message->current_time);
        LOGI(TAG, "Call timesync in Battery level");
        lv_msg_send(MSG_TIME_SYNCED, NULL);
#if 0
        enable_buttons();
#endif
      }
    } break;
    case START_FLOW: {
      send_command_data(RESPONSE_COMMAND, &message->sender_type, sizeof(message->sender_type));
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
#if 0
      int zone_id = message->deviceId;
      if (zone_id >= 1 && zone_id <= 6) {
        char op_msg[128] = { 0 };
        lvgl_acquire();
        set_zone_status(zone_id, true);
        set_zone_number(zone_id, true);
        disable_start_button();
        snprintf(op_msg, sizeof(op_msg), "Start to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
        add_operation_list(op_msg);
        lvgl_release();
      } else {
        LOGW(TAG, "Got invalid zone_id for start flow = [%d]", zone_id);
      }
#endif
    } break;
    case ZONE_COMPLETE: {
      send_command_data(RESPONSE_COMMAND, &message->sender_type, sizeof(message->sender_type));
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
#if 0
      int zone_id = message->deviceId;
      if (zone_id >= 1 && zone_id <= 6) {
        lvgl_acquire();
        char op_msg[128] = { 0 };
        int flow_value = message->flow_value;
        set_zone_status(zone_id, false);
        set_zone_number(zone_id, false);
        set_zone_flow_value(zone_id, flow_value);
        snprintf(op_msg, sizeof(op_msg), "Stop to irrigation of zone[%d] at %s\n", zone_id, get_current_timestamp());
        add_operation_list(op_msg);
        lvgl_release();
      }
#endif
    } break;
    case ALL_COMPLETE: {
      send_command_data(RESPONSE_COMMAND, &message->sender_type, sizeof(message->sender_type));
      // Update UI screen
      lv_msg_send(MSG_IRRIGATION_STATUS, message);
#if 0
      enable_start_button();
#endif
    } break;
    case SET_SLEEP: {
      uint64_t wakeup_time_sec = message->remain_time_sleep;
      LOGI(TAG, "Entering sleep mode wtih time = %llus", wakeup_time_sec);
      esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000);
      esp_deep_sleep_start();
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

  ctrl_msg_queue = xQueueCreate(CTRL_MSG_QUEUE_LEN, sizeof(irrigation_message_t));
  if (ctrl_msg_queue == NULL) {
    return;
  }

  xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 0);
}
