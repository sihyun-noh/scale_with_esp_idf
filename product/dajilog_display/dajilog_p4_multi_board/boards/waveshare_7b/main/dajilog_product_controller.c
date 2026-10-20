#include "dajilog_system_core.h"
#include "dajilog_product_controller.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "dajilog_board.h"
#include "dajilog_can.h"
#include "dajilog_can_device_manager.h"
#include "dajilog_settings.h"
#include "dajilog_ui_bridge.h"
#include "dajilog_ui_msg.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define SYSTEM_QUEUE_LENGTH  16U
#define STORAGE_QUEUE_LENGTH 8U

typedef enum {
  CORE_MESSAGE_UI_COMMAND = 0,
  CORE_MESSAGE_STORAGE_RESULT,
  CORE_MESSAGE_VEHICLE_UPDATE,
  CORE_MESSAGE_MOTOR_UPDATE,
} core_message_kind_t;

typedef enum {
  STORAGE_SAVE_SETTINGS = 0,
  STORAGE_DISCARD_SETTINGS,
} storage_operation_t;

typedef struct {
  storage_operation_t operation;
  uint32_t sequence;
} storage_request_t;

typedef struct {
  core_message_kind_t kind;
  uint32_t topic;
  uint32_t sequence;
  union {
    esp_err_t storage_error;
    dajilog_vehicle_snapshot_t vehicle;
    struct {
      size_t index;
      dajilog_motor_snapshot_t motor;
    } motor;
    dajilog_manual_motor_command_t manual_motor;
  } data;
} core_message_t;

static const char* TAG = "DajilogProduct";
static QueueHandle_t s_system_queue;
static QueueHandle_t s_storage_queue;
static SemaphoreHandle_t s_snapshot_lock;
static TaskHandle_t s_system_task;
static TaskHandle_t s_storage_task;
static dajilog_dashboard_snapshot_t s_snapshot;
static bool s_started;
static dajilog_motor_backend_t s_motor_backend;
static bool s_motor_backend_registered;

static esp_err_t start_can_service(void) {
  const dajilog_board_can_config_t* board_can = dajilog_board_get_can_config();
  if (board_can == NULL || !board_can->supported) {
    ESP_LOGI(TAG, "CAN service is not supported on this board");
    return ESP_ERR_NOT_SUPPORTED;
  }
  if (board_can->tx_gpio < 0 || board_can->rx_gpio < 0) {
    return ESP_ERR_INVALID_ARG;
  }

  const dajilog_can_config_t config = {
    .tx_gpio = board_can->tx_gpio,
    .rx_gpio = board_can->rx_gpio,
    .bitrate = board_can->bitrate,
    .tx_queue_length = 16,
    .rx_queue_length = 32,
    .filter = dajilog_can_filter_accept_all(),
  };
  esp_err_t error = dajilog_can_init(&config);
  if (error == ESP_OK) {
    error = dajilog_can_start();
  }
  if (error == ESP_OK) {
    error = dajilog_can_device_manager_start();
  }
  return error;
}

static const char* state_name(dajilog_operation_state_t state) {
  static const char* names[] = {
    "BOOT", "INITIALIZING", "READY", "RUN_AUTO", "RUN_MANUAL", "STOPPING", "FAULT",
  };
  return (unsigned)state < (sizeof(names) / sizeof(names[0])) ? names[state] : "UNKNOWN";
}

static dajilog_ui_result_t map_result(esp_err_t error) {
  if (error == ESP_OK) {
    return DAJILOG_UI_RESULT_OK;
  }
  if (error == ESP_ERR_INVALID_ARG) {
    return DAJILOG_UI_RESULT_INVALID;
  }
  if (error == ESP_ERR_INVALID_STATE || error == ESP_ERR_NOT_SUPPORTED) {
    return DAJILOG_UI_RESULT_REJECTED;
  }
  if (error == ESP_ERR_TIMEOUT) {
    return DAJILOG_UI_RESULT_TIMEOUT;
  }
  return DAJILOG_UI_RESULT_INTERNAL_ERROR;
}

static void publish_u32(dajilog_ui_msg_kind_t kind, uint32_t topic, uint32_t sequence, dajilog_ui_result_t result,
                        uint32_t value) {
  dajilog_ui_msg_t message;
  dajilog_ui_msg_init(&message, kind, topic);
  message.sequence = sequence;
  message.result = result;
  message.payload.type = DAJILOG_UI_VALUE_U32;
  message.payload.value.u32 = value;
  const esp_err_t error = dajilog_ui_bridge_publish(&message);
  if (error != ESP_OK) {
    ESP_LOGW(TAG, "publish failed: 0x%08lx (%s)", (unsigned long)topic, esp_err_to_name(error));
  }
}

static void publish_snapshot(void) {
  uint32_t revision;
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  if (++s_snapshot.revision == 0) {
    s_snapshot.revision = 1;
  }
  s_snapshot.timestamp_us = esp_timer_get_time();
  revision = s_snapshot.revision;
  xSemaphoreGive(s_snapshot_lock);
  publish_u32(DAJILOG_UI_MSG_STATE, DAJILOG_UI_STATE_DASHBOARD_SNAPSHOT, 0, DAJILOG_UI_RESULT_OK, revision);
}

static void publish_result(uint32_t topic, uint32_t sequence, esp_err_t error) {
  publish_u32(DAJILOG_UI_MSG_RESULT, topic, sequence, map_result(error), (uint32_t)error);
}

static void set_state(dajilog_operation_state_t state) {
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  s_snapshot.operation_state = state;
  xSemaphoreGive(s_snapshot_lock);
  ESP_LOGI(TAG, "FSM state: %s", state_name(state));
}

static void set_alarm(dajilog_alarm_severity_t severity, uint32_t code, const char* text) {
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  s_snapshot.alarm_severity = severity;
  s_snapshot.alarm_code = code;
  snprintf(s_snapshot.alarm_text, sizeof(s_snapshot.alarm_text), "%s", text != NULL ? text : "");
  xSemaphoreGive(s_snapshot_lock);
}

static esp_err_t bridge_command_handler(const dajilog_ui_msg_t* command, void* context) {
  QueueHandle_t queue = (QueueHandle_t)context;
  if (command == NULL || queue == NULL || command->kind != DAJILOG_UI_MSG_COMMAND) {
    return ESP_ERR_INVALID_ARG;
  }
  const core_message_t message = {
    .kind = CORE_MESSAGE_UI_COMMAND,
    .topic = command->topic,
    .sequence = command->sequence,
  };
  core_message_t queued_message = message;
  if (command->topic == DAJILOG_UI_CMD_MOTOR_MANUAL) {
    if (command->payload.type != DAJILOG_UI_VALUE_BYTES ||
        command->payload.value.bytes.size != sizeof(dajilog_manual_motor_command_t)) {
      return ESP_ERR_INVALID_ARG;
    }
    memcpy(&queued_message.data.manual_motor, command->payload.value.bytes.data,
           sizeof(queued_message.data.manual_motor));
  }
  return xQueueSend(queue, &queued_message, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

static esp_err_t request_storage(storage_operation_t operation, uint32_t sequence) {
  const storage_request_t request = {
    .operation = operation,
    .sequence = sequence,
  };
  return xQueueSend(s_storage_queue, &request, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

static void process_command(const core_message_t* message) {
  esp_err_t error = ESP_OK;
  bool deferred = false;

  switch (message->topic) {
    case DAJILOG_UI_CMD_STOP:
      set_state(DAJILOG_OPERATION_STOPPING);
      publish_snapshot();
      if (s_motor_backend_registered && s_motor_backend.stop != NULL) {
        error = s_motor_backend.stop(s_motor_backend.context);
      }
      set_state(DAJILOG_OPERATION_READY);
      set_alarm(DAJILOG_ALARM_NONE, 0, "");
      break;
    case DAJILOG_UI_CMD_MOTOR_MANUAL:
      error = s_motor_backend_registered && s_motor_backend.command != NULL
                  ? s_motor_backend.command(&message->data.manual_motor,
                                            s_motor_backend.context)
                  : ESP_ERR_NOT_SUPPORTED;
      if (error == ESP_OK) {
        const bool running = s_motor_backend.is_running != NULL &&
                             s_motor_backend.is_running(s_motor_backend.context);
        set_state(running ? DAJILOG_OPERATION_RUN_MANUAL
                          : DAJILOG_OPERATION_READY);
        set_alarm(DAJILOG_ALARM_NONE, 0, "");
      } else {
        set_alarm(DAJILOG_ALARM_WARNING, 0x3101U,
                  "Manual motor 1 command rejected");
      }
      break;
    case DAJILOG_UI_CMD_ACK_ALARM: set_alarm(DAJILOG_ALARM_NONE, 0, ""); break;
    case DAJILOG_UI_CMD_SETTINGS_APPLY:
      error = request_storage(STORAGE_SAVE_SETTINGS, message->sequence);
      deferred = error == ESP_OK;
      break;
    case DAJILOG_UI_CMD_SETTINGS_DISCARD:
      error = request_storage(STORAGE_DISCARD_SETTINGS, message->sequence);
      deferred = error == ESP_OK;
      break;
    case DAJILOG_UI_CMD_START_AUTO:
    case DAJILOG_UI_CMD_START_MANUAL:
      error = ESP_ERR_NOT_SUPPORTED;
      set_alarm(DAJILOG_ALARM_WARNING, 0x1001U, "Motor control is not connected");
      break;
    default: error = ESP_ERR_INVALID_ARG; break;
  }

  if (!deferred) {
    publish_result(DAJILOG_UI_STATE_COMMAND_RESULT, message->sequence, error);
    publish_snapshot();
  }
}

static void system_task(void* context) {
  (void)context;
  set_state(DAJILOG_OPERATION_READY);
  publish_snapshot();

  core_message_t message;
  while (xQueueReceive(s_system_queue, &message, portMAX_DELAY) == pdTRUE) {
    switch (message.kind) {
      case CORE_MESSAGE_UI_COMMAND: process_command(&message); break;
      case CORE_MESSAGE_STORAGE_RESULT:
        if (message.data.storage_error == ESP_OK) {
          set_alarm(DAJILOG_ALARM_NONE, 0, "");
        } else {
          set_alarm(DAJILOG_ALARM_WARNING, 0x2001U, "Settings operation failed");
        }
        publish_result(DAJILOG_UI_STATE_SETTINGS_RESULT, message.sequence, message.data.storage_error);
        publish_snapshot();
        break;
      case CORE_MESSAGE_VEHICLE_UPDATE:
        xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
        s_snapshot.vehicle = message.data.vehicle;
        s_snapshot.can_online = message.data.vehicle.online;
        for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
          s_snapshot.can_online = s_snapshot.can_online || s_snapshot.motors[i].online;
        }
        xSemaphoreGive(s_snapshot_lock);
        publish_snapshot();
        break;
      case CORE_MESSAGE_MOTOR_UPDATE:
        if (message.data.motor.index < DAJILOG_MOTOR_CHANNEL_COUNT) {
          xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
          s_snapshot.motors[message.data.motor.index] = message.data.motor.motor;
          s_snapshot.can_online = s_snapshot.vehicle.online;
          for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
            s_snapshot.can_online = s_snapshot.can_online || s_snapshot.motors[i].online;
          }
          xSemaphoreGive(s_snapshot_lock);
          publish_snapshot();
        }
        break;
      default: break;
    }
  }
}

static void storage_task(void* context) {
  (void)context;
  storage_request_t request;
  while (xQueueReceive(s_storage_queue, &request, portMAX_DELAY) == pdTRUE) {
    esp_err_t error = request.operation == STORAGE_SAVE_SETTINGS ? dajilog_settings_save_all()
                                                                 : dajilog_settings_discard_all_drafts();
    const core_message_t result = {
      .kind = CORE_MESSAGE_STORAGE_RESULT,
      .sequence = request.sequence,
      .data.storage_error = error,
    };
    if (xQueueSend(s_system_queue, &result, pdMS_TO_TICKS(100)) != pdTRUE) {
      ESP_LOGE(TAG, "System Queue full; storage result dropped");
    }
  }
}

static esp_err_t product_start(void* context) {
  (void)context;
  if (s_started) {
    return ESP_OK;
  }
  esp_err_t error = dajilog_settings_init();
  if (error != ESP_OK) {
    return error;
  }

  s_snapshot_lock = xSemaphoreCreateMutex();
  s_system_queue = xQueueCreate(SYSTEM_QUEUE_LENGTH, sizeof(core_message_t));
  s_storage_queue = xQueueCreate(STORAGE_QUEUE_LENGTH, sizeof(storage_request_t));
  if (s_snapshot_lock == NULL || s_system_queue == NULL || s_storage_queue == NULL) {
    return ESP_ERR_NO_MEM;
  }

  memset(&s_snapshot, 0, sizeof(s_snapshot));
  s_snapshot.operation_state = DAJILOG_OPERATION_INITIALIZING;
  s_snapshot.work_mode = DAJILOG_WORK_MODE_AUTO;
  s_snapshot.vehicle.target_speed_kmh = 8.0f;
  snprintf(s_snapshot.recipe_name, sizeof(s_snapshot.recipe_name), "Default recipe");
  for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
    s_snapshot.motors[i].state = DAJILOG_MOTOR_DISABLED;
  }

  const dajilog_ui_bridge_config_t bridge_config = {
    .command_handler = bridge_command_handler,
    .command_context = s_system_queue,
  };
  error = dajilog_ui_bridge_init(&bridge_config);
  if (error != ESP_OK) {
    return error;
  }
  if (xTaskCreate(storage_task, "dajilog_storage", 4096, NULL, 7, &s_storage_task) != pdPASS) {
    return ESP_ERR_NO_MEM;
  }
  if (xTaskCreate(system_task, "dajilog_system", 4096, NULL, 9, &s_system_task) != pdPASS) {
    vTaskDelete(s_storage_task);
    s_storage_task = NULL;
    return ESP_ERR_NO_MEM;
  }
  s_started = true;
  error = start_can_service();
  if (error != ESP_OK && error != ESP_ERR_NOT_SUPPORTED) {
    ESP_LOGE(TAG, "CAN service start failed: %s", esp_err_to_name(error));
    set_alarm(DAJILOG_ALARM_WARNING, 0x3001U, "CAN service start failed");
  } else if (error == ESP_OK) {
    if (s_motor_backend_registered && s_motor_backend.start != NULL &&
        s_motor_backend.start(s_motor_backend.context) != ESP_OK) {
      ESP_LOGE(TAG, "Motor backend start failed");
      set_alarm(DAJILOG_ALARM_WARNING, 0x3002U,
                "Motor backend start failed");
    }
  }
  ESP_LOGI(TAG, "7B product controller started");
  return ESP_OK;
}

static esp_err_t product_register_motor_backend(
    const dajilog_motor_backend_t* backend, void* context) {
  (void)context;
  if (backend == NULL || backend->command == NULL || backend->stop == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  if (s_started) {
    return ESP_ERR_INVALID_STATE;
  }
  s_motor_backend = *backend;
  s_motor_backend_registered = true;
  return ESP_OK;
}

static bool product_is_started(void* context) {
  (void)context;
  return s_started;
}

static esp_err_t product_get_snapshot(dajilog_dashboard_snapshot_t* snapshot,
                                      void* context) {
  (void)context;
  if (!s_started) {
    return ESP_ERR_INVALID_STATE;
  }
  if (snapshot == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  *snapshot = s_snapshot;
  xSemaphoreGive(s_snapshot_lock);
  return ESP_OK;
}

static esp_err_t product_update_vehicle(float speed_kmh, bool online,
                                        uint32_t signal_age_ms,
                                        void* context) {
  (void)context;
  if (!s_started || !isfinite(speed_kmh) || speed_kmh < 0.0f) {
    return ESP_ERR_INVALID_ARG;
  }
  const core_message_t message = {
        .kind = CORE_MESSAGE_VEHICLE_UPDATE,
        .data.vehicle = {
            .speed_kmh = speed_kmh,
            .target_speed_kmh = 8.0f,
            .signal_age_ms = signal_age_ms,
            .online = online,
        },
    };
  return xQueueSend(s_system_queue, &message, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

static esp_err_t product_update_motor(size_t index,
                                      const dajilog_motor_snapshot_t* motor,
                                      void* context) {
  (void)context;
  if (!s_started || index >= DAJILOG_MOTOR_CHANNEL_COUNT || motor == NULL) {
    return ESP_ERR_INVALID_ARG;
  }
  const core_message_t message = {
    .kind = CORE_MESSAGE_MOTOR_UPDATE,
    .data.motor = { .index = index, .motor = *motor },
  };
  return xQueueSend(s_system_queue, &message, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

esp_err_t dajilog_product_controller_register(void) {
  const dajilog_system_core_implementation_t implementation = {
    .start = product_start,
    .is_started = product_is_started,
    .get_snapshot = product_get_snapshot,
    .update_vehicle = product_update_vehicle,
    .update_motor = product_update_motor,
    .register_motor_backend = product_register_motor_backend,
    .context = NULL,
  };
  return dajilog_system_core_register_implementation(&implementation);
}
