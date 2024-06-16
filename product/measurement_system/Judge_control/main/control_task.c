
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sysevent.h"
#include "log.h"
#include "scale_read_485.h"
#include "event_ids.h"

#include <string.h>

#define CTRL_MSG_QUEUE_LEN 16

static const char *TAG = "ctrl_task";

static TaskHandle_t control_handle = NULL;
static QueueHandle_t ctrl_msg_queue = NULL;

static bool _send_msg_event(Common_data_t *msg) {
  return ctrl_msg_queue && (xQueueSend(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

static bool _get_msg_event(Common_data_t *msg) {
  return ctrl_msg_queue && (xQueueReceive(ctrl_msg_queue, msg, portMAX_DELAY) == pdPASS);
}

void send_msg_to_ctrl_task(void *msg, size_t msg_len) {
  Common_data_t message = { 0 };

  if (msg && (msg_len == sizeof(Common_data_t))) {
    memcpy(&message, msg, msg_len);
    if (!_send_msg_event(&message)) {
      LOGI(TAG, "Failed to send ctrl messag event!!!");
    }
  }
}

static void control_task(void *pvParameter) {
  Common_data_t weight_data;
  char str[100] = { 0 };
  int res = 0;
  for (;;) {
    // LOGE(TAG, "Failed to receive ctrl message form queue");
    // LOGI(TAG, "start firmware");
    memset(&weight_data, 0x00, sizeof(weight_data));
    res = indicator_INNOTEM_T25_data(&weight_data);
    // weight, unit, zero, stable, sign, trash
    // 자릿수확인 1 -> 10 변할 때 1자리에서 2자리로
    if (res == -1) {
      sysevent_set(WEIGHT_DATA_RES_EVENT, &res);
    } else {
      snprintf(str, sizeof(str), "%10s,%d,%d,%d,%d,%d", weight_data.weight_data, weight_data.spec.unit,
               weight_data.event[STATE_ZERO_EVENT], weight_data.event[STATE_STABLE_EVENT],
               weight_data.event[STATE_SIGN_EVENT], weight_data.event[STATE_TRASH_CHECK_EVENT]);
      sysevent_set(WEIGHT_DATA_EVENT, str);
      sysevent_set(WEIGHT_DATA_RES_EVENT, &res);
    }
    vTaskDelay(500);
    continue;
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
  ctrl_msg_queue = xQueueCreate(CTRL_MSG_QUEUE_LEN, sizeof(Common_data_t));
  if (ctrl_msg_queue == NULL) {
    return;
  }

  xTaskCreatePinnedToCore(control_task, "control_task", stack_size, NULL, task_priority, &control_handle, 0);
}
