#include "dajilog_system_core.h"
#include "dajilog_product_controller.h"
#include "dajilog_product_settings.h"
#include "dajilog_driver_policy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "dajilog_board.h"
#include "dajilog_can.h"
#include "dajilog_can_device_manager.h"
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
  uint32_t epoch;
  int64_t posted_us;
  union {
    esp_err_t storage_error;
    dajilog_vehicle_snapshot_t vehicle;
    struct {
      size_t index;
      dajilog_motor_snapshot_t motor;
    } motor;
    dajilog_motor_voltage_command_t motor_voltage;
    dajilog_driver_setting_t driver;
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
static uint32_t s_stop_epoch;
static bool s_stop_requested;
static bool s_test_active;
static int64_t s_test_deadline;
static dajilog_motor_voltage_command_t s_test_command;
static int64_t s_command_time[DAJILOG_MOTOR_CHANNEL_COUNT];
static uint8_t s_stop_pending_mask;
static uint8_t s_unconfirmed_output_mask;
static int64_t s_stop_deadline;

/** @brief Validate both channels without changing output or the applied snapshot. */
static esp_err_t validate_driver(const dajilog_driver_setting_t *driver) {
  if (driver->mode == 1) return ESP_ERR_NOT_SUPPORTED;
  return dajilog_driver_setting_valid(driver) ? ESP_OK : ESP_ERR_INVALID_ARG;
}

/** @brief Require live, fault-free feedback before authorizing energization. */
static bool channel_ready(size_t index) {
  return index < DAJILOG_MOTOR_CHANNEL_COUNT &&
         dajilog_driver_feedback_ready(&s_snapshot.motors[index], esp_timer_get_time());
}

/** @brief Track requested voltage only after backend acceptance (not physical confirmation). */
static esp_err_t command_voltage(const dajilog_motor_voltage_command_t *command) {
  if (!s_motor_backend_registered || s_motor_backend.command == NULL)
    return ESP_ERR_NOT_SUPPORTED;
  esp_err_t error = s_motor_backend.command(command, s_motor_backend.context);
  if (error == ESP_OK) {
    xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
    s_snapshot.motors[command->motor_index].command_decivolt = command->final_decivolt;
    s_snapshot.motors[command->motor_index].command_pending = true;
    s_command_time[command->motor_index] = esp_timer_get_time();
    if (command->final_decivolt != 0) s_unconfirmed_output_mask |= 1U << command->motor_index;
    xSemaphoreGive(s_snapshot_lock);
  }
  return error;
}

/** @brief Initialize the board CAN driver and start the device manager. */
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

/** @brief Return a log-friendly name for a product operation state. */
static const char* state_name(dajilog_operation_state_t state) {
  static const char* names[] = {
    "BOOT", "INITIALIZING", "READY", "RUN_AUTO", "RUN_MANUAL", "STOPPING", "FAULT",
  };
  return (unsigned)state < (sizeof(names) / sizeof(names[0])) ? names[state] : "UNKNOWN";
}

/** @brief Convert an ESP-IDF result into the stable UI result contract. */
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

/** @brief Publish a scalar state or result message through the UI Bridge. */
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

/** @brief Advance the dashboard revision and notify UI subscribers. */
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

/** @brief Publish an asynchronous command result with its request sequence. */
static void publish_result(uint32_t topic, uint32_t sequence, esp_err_t error) {
  publish_u32(DAJILOG_UI_MSG_RESULT, topic, sequence, map_result(error), (uint32_t)error);
}

/** @brief Update the FSM state stored in the protected dashboard snapshot. */
static void set_state(dajilog_operation_state_t state) {
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  s_snapshot.operation_state = state;
  xSemaphoreGive(s_snapshot_lock);
  ESP_LOGI(TAG, "FSM state: %s", state_name(state));
}

/** @brief Replace the current alarm fields in the dashboard snapshot. */
static void set_alarm(dajilog_alarm_severity_t severity, uint32_t code, const char* text) {
  xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
  s_snapshot.alarm_severity = severity;
  s_snapshot.alarm_code = code;
  snprintf(s_snapshot.alarm_text, sizeof(s_snapshot.alarm_text), "%s", text != NULL ? text : "");
  xSemaphoreGive(s_snapshot_lock);
}

/**
 * @brief Copy a UI command into the System Task queue.
 *
 * This callback runs in the command producer's context and never performs
 * motor, storage, or FSM work directly.
 */
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
  queued_message.epoch = __atomic_load_n(&s_stop_epoch, __ATOMIC_SEQ_CST);
  queued_message.posted_us = esp_timer_get_time();
  if (command->topic == DAJILOG_UI_CMD_STOP) {
    __atomic_add_fetch(&s_stop_epoch, 1, __ATOMIC_SEQ_CST);
    __atomic_store_n(&s_stop_requested, true, __ATOMIC_SEQ_CST);
    return ESP_OK; /* STOP cannot be lost when the ordinary queue is full. */
  }
  if (command->topic == DAJILOG_UI_CMD_DRIVER_APPLY) {
    if (command->payload.type != DAJILOG_UI_VALUE_BYTES ||
        command->payload.value.bytes.size != sizeof(dajilog_driver_setting_t))
      return ESP_ERR_INVALID_ARG;
    memcpy(&queued_message.data.driver, command->payload.value.bytes.data,
           sizeof(queued_message.data.driver));
  }
  if (command->topic == DAJILOG_UI_CMD_MOTOR_VOLTAGE_TEST) {
    if (command->payload.type != DAJILOG_UI_VALUE_BYTES ||
        command->payload.value.bytes.size != sizeof(dajilog_motor_voltage_command_t)) {
      return ESP_ERR_INVALID_ARG;
    }
    memcpy(&queued_message.data.motor_voltage, command->payload.value.bytes.data,
           sizeof(queued_message.data.motor_voltage));
  }
  return xQueueSend(queue, &queued_message, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

/** @brief Enqueue a settings operation for exclusive execution by Storage Task. */
static esp_err_t request_storage(storage_operation_t operation, uint32_t sequence) {
  const storage_request_t request = {
    .operation = operation,
    .sequence = sequence,
  };
  return xQueueSend(s_storage_queue, &request, 0) == pdTRUE ? ESP_OK : ESP_ERR_TIMEOUT;
}

/**
 * @brief Apply FSM policy to one UI command received by System Task.
 *
 * Long-running storage work is deferred to Storage Task. Immediate motor and
 * alarm decisions remain serialized in System Task.
 */
static void process_command(const core_message_t* message) {
  esp_err_t error = ESP_OK;
  bool deferred = false;
  if (message->topic != DAJILOG_UI_CMD_STOP &&
      message->epoch != __atomic_load_n(&s_stop_epoch, __ATOMIC_SEQ_CST)) {
    publish_result(DAJILOG_UI_STATE_COMMAND_RESULT, message->sequence, ESP_ERR_INVALID_STATE);
    return;
  }

  switch (message->topic) {
    case DAJILOG_UI_CMD_STOP:
      s_test_active = false;
      s_stop_pending_mask = s_unconfirmed_output_mask;
      for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        if (s_snapshot.motors[i].command_decivolt != 0)
          s_stop_pending_mask |= 1U << i;
      }
      s_stop_deadline = esp_timer_get_time() + 3000000;
      set_state(DAJILOG_OPERATION_STOPPING);
      publish_snapshot();
      if (s_motor_backend_registered && s_motor_backend.stop != NULL) {
        error = s_motor_backend.stop(s_motor_backend.context);
      }
      if (error == ESP_OK) {
        xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
        for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
          s_snapshot.motors[i].command_decivolt = 0;
          s_snapshot.motors[i].command_pending = true;
          s_command_time[i] = esp_timer_get_time();
        }
        xSemaphoreGive(s_snapshot_lock);
      }
      xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
      s_snapshot.work_mode = (s_snapshot.drivers[0].mode == 2 || s_snapshot.drivers[1].mode == 2)
                                ? DAJILOG_WORK_MODE_MANUAL : DAJILOG_WORK_MODE_AUTO;
      xSemaphoreGive(s_snapshot_lock);
      set_state(error != ESP_OK ? DAJILOG_OPERATION_FAULT :
                s_stop_pending_mask ? DAJILOG_OPERATION_STOPPING : DAJILOG_OPERATION_READY);
      if (error != ESP_OK) s_stop_pending_mask = 0;
      set_alarm(error == ESP_OK ? DAJILOG_ALARM_NONE : DAJILOG_ALARM_CRITICAL,
                error == ESP_OK ? 0 : 0x3102U,
                error == ESP_OK ? "" : "STOP transmission failed; verify motor power");
      break;
    case DAJILOG_UI_CMD_DRIVER_APPLY:
      error = validate_driver(&message->data.driver);
      if (error == ESP_OK && (s_snapshot.operation_state != DAJILOG_OPERATION_READY || s_test_active))
        error = ESP_ERR_INVALID_STATE;
      if (error == ESP_OK && message->data.driver.mode != 0 &&
          (!channel_ready(message->data.driver.driver_index * 2U) ||
           !channel_ready(message->data.driver.driver_index * 2U + 1U)))
        error = ESP_ERR_INVALID_STATE;
      if (error == ESP_OK) {
        xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
        const size_t index = message->data.driver.driver_index;
        uint32_t revision = s_snapshot.drivers[index].revision + 1U;
        s_snapshot.drivers[index] = message->data.driver;
        s_snapshot.drivers[index].revision = revision;
        xSemaphoreGive(s_snapshot_lock);
      }
      break;
    case DAJILOG_UI_CMD_MOTOR_VOLTAGE_TEST:
      if (message->data.motor_voltage.motor_index >= DAJILOG_MOTOR_CHANNEL_COUNT ||
          message->posted_us + 500000 < esp_timer_get_time() ||
          (!s_test_active && s_snapshot.operation_state != DAJILOG_OPERATION_READY) ||
          (s_test_active && message->data.motor_voltage.motor_index != s_test_command.motor_index) ||
          !channel_ready(message->data.motor_voltage.motor_index)) {
        error = ESP_ERR_INVALID_STATE;
        break;
      }
      if (s_test_active &&
          s_test_command.final_decivolt == message->data.motor_voltage.final_decivolt &&
          s_test_command.output_step == message->data.motor_voltage.output_step &&
          s_test_command.trim_decivolt == message->data.motor_voltage.trim_decivolt) {
        s_test_deadline = message->posted_us + 500000;
        return;
      }
      error = command_voltage(&message->data.motor_voltage);
      if (error == ESP_OK) {
        s_test_command = message->data.motor_voltage;
        s_test_active = true;
        s_test_deadline = message->posted_us + 500000;
        xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
        s_snapshot.work_mode = DAJILOG_WORK_MODE_TEST;
        xSemaphoreGive(s_snapshot_lock);
        const bool running = s_motor_backend.is_running != NULL &&
                             s_motor_backend.is_running(s_motor_backend.context);
        set_state(running ? DAJILOG_OPERATION_RUN_MANUAL
                          : DAJILOG_OPERATION_READY);
        set_alarm(DAJILOG_ALARM_NONE, 0, "");
      } else {
        set_alarm(DAJILOG_ALARM_WARNING, 0x3101U,
                  "Motor voltage test command rejected");
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
    case DAJILOG_UI_CMD_START_MANUAL:
      if (s_snapshot.operation_state != DAJILOG_OPERATION_READY || s_test_active ||
          !s_motor_backend_registered) { error = ESP_ERR_INVALID_STATE; break; }
      {
        bool selected = false;
        for (size_t d = 0; d < 2; ++d) {
          if (s_snapshot.drivers[d].mode == 0) continue;
          selected = true;
          if (s_snapshot.drivers[d].mode != 2 || !channel_ready(d * 2) || !channel_ready(d * 2 + 1))
            error = ESP_ERR_INVALID_STATE;
        }
        if (!selected) error = ESP_ERR_INVALID_STATE;
        if (error != ESP_OK) break;
        for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT && error == ESP_OK; ++i) {
          const dajilog_driver_setting_t *driver = &s_snapshot.drivers[i / 2];
          if (driver->mode == 0) continue;
          const dajilog_motor_voltage_command_t command = {
            .motor_index = i, .output_step = driver->step[i % 2],
            .trim_decivolt = driver->fine_decivolt[i % 2],
            .final_decivolt = driver->step[i % 2] * 5U + driver->fine_decivolt[i % 2],
          };
          error = command_voltage(&command);
        }
        if (error != ESP_OK) {
          (void)s_motor_backend.stop(s_motor_backend.context);
          set_state(DAJILOG_OPERATION_FAULT);
          set_alarm(DAJILOG_ALARM_CRITICAL, 0x3103U, "RUN failed; verify all outputs stopped");
        } else {
          xSemaphoreTake(s_snapshot_lock, portMAX_DELAY);
          s_snapshot.work_mode = DAJILOG_WORK_MODE_MANUAL;
          xSemaphoreGive(s_snapshot_lock);
          set_state(DAJILOG_OPERATION_RUN_MANUAL);
        }
      }
      break;
    case DAJILOG_UI_CMD_START_AUTO:
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

/**
 * @brief Own the product FSM and merge UI, storage, vehicle, and motor events.
 *
 * This is the only task that mutates the logical dashboard state in response
 * to queued events.
 */
static void system_task(void* context) {
  (void)context;
  set_state(DAJILOG_OPERATION_READY);
  publish_snapshot();

  core_message_t message;
  for (;;) {
    bool stop = __atomic_exchange_n(&s_stop_requested, false, __ATOMIC_SEQ_CST);
    if (s_test_active && esp_timer_get_time() >= s_test_deadline) stop = true;
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
      const bool active = s_test_active ? i == s_test_command.motor_index :
          s_snapshot.operation_state == DAJILOG_OPERATION_RUN_MANUAL &&
          s_snapshot.drivers[i / 2].mode == 2;
      if (active && !channel_ready(i)) stop = true;
      if (active && s_snapshot.motors[i].command_decivolt != 0 &&
          esp_timer_get_time() - s_command_time[i] > 3000000 &&
          (s_snapshot.motors[i].command_pending ||
           s_snapshot.motors[i].readback_timestamp_us < s_command_time[i]))
        stop = true;
    }
    if (s_stop_pending_mask) {
      for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        const dajilog_motor_snapshot_t *motor = &s_snapshot.motors[i];
        if (motor->command_readback_valid && motor->command_readback_decivolt == 0 &&
            motor->readback_timestamp_us >= s_command_time[i]) {
          s_stop_pending_mask &= ~(1U << i);
          s_unconfirmed_output_mask &= ~(1U << i);
        }
      }
      if (!s_stop_pending_mask) {
        set_state(DAJILOG_OPERATION_READY);
        publish_snapshot();
      } else if (esp_timer_get_time() >= s_stop_deadline) {
        s_stop_pending_mask = 0;
        set_state(DAJILOG_OPERATION_FAULT);
        set_alarm(DAJILOG_ALARM_CRITICAL, 0x3104U, "STOP not confirmed; isolate motor power");
        publish_snapshot();
      }
    }
    if (stop) {
      __atomic_add_fetch(&s_stop_epoch, 1, __ATOMIC_SEQ_CST);
      const core_message_t request = {.topic = DAJILOG_UI_CMD_STOP};
      process_command(&request);
    }
    if (xQueueReceive(s_system_queue, &message, pdMS_TO_TICKS(50)) != pdTRUE) continue;
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
          const uint16_t requested = s_snapshot.motors[message.data.motor.index].command_decivolt;
          s_snapshot.motors[message.data.motor.index] = message.data.motor.motor;
          s_snapshot.motors[message.data.motor.index].command_decivolt = requested;
          s_snapshot.motors[message.data.motor.index].command_pending =
              !message.data.motor.motor.command_readback_valid ||
              message.data.motor.motor.command_readback_decivolt != requested;
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

/**
 * @brief Execute settings save/discard requests outside LVGL and System Task.
 *
 * Completion is returned to System Task as `CORE_MESSAGE_STORAGE_RESULT`.
 */
static void storage_task(void* context) {
  (void)context;
  storage_request_t request;
  while (xQueueReceive(s_storage_queue, &request, portMAX_DELAY) == pdTRUE) {
    esp_err_t error = request.operation == STORAGE_SAVE_SETTINGS
                          ? dajilog_product_settings_apply()
                          : dajilog_product_settings_discard();
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

/**
 * @brief Initialize product settings, queues, tasks, UI Bridge, and CAN service.
 *
 * Called through the common System Core facade after all product backends have
 * been registered.
 */
static esp_err_t product_start(void* context) {
  (void)context;
  if (s_started) {
    return ESP_OK;
  }
  esp_err_t error = dajilog_product_settings_init();
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

/** @brief Install the board-specific motor callbacks before controller start. */
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

/** @brief Report whether product queues and worker tasks have been started. */
static bool product_is_started(void* context) {
  (void)context;
  return s_started;
}

/** @brief Copy the mutex-protected dashboard snapshot for a reader. */
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

/** @brief Queue a decoded vehicle-speed update for System Task processing. */
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

/** @brief Queue one decoded motor state update for System Task processing. */
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

/** @brief Forward a draft read through the product settings service. */
static esp_err_t product_get_settings_draft(
    dajilog_product_settings_snapshot_t* settings, void* context) {
  (void)context;
  return dajilog_product_settings_get_draft(settings);
}

/** @brief Forward validated ridge input to the product settings draft. */
static esp_err_t product_stage_ridge_settings(
    const dajilog_ridge_settings_t* settings, void* context) {
  (void)context;
  return dajilog_product_settings_stage_ridge(settings);
}

/** @brief Forward validated prescription input to the product settings draft. */
static esp_err_t product_stage_prescription_settings(
    const dajilog_prescription_settings_t* settings, void* context) {
  (void)context;
  return dajilog_product_settings_stage_prescription(settings);
}

/** @brief Bind the 7B implementation callbacks to the common System Core API. */
esp_err_t dajilog_product_controller_register(void) {
  const dajilog_system_core_implementation_t implementation = {
    .start = product_start,
    .is_started = product_is_started,
    .get_snapshot = product_get_snapshot,
    .update_vehicle = product_update_vehicle,
    .update_motor = product_update_motor,
    .register_motor_backend = product_register_motor_backend,
    .get_settings_draft = product_get_settings_draft,
    .stage_ridge_settings = product_stage_ridge_settings,
    .stage_prescription_settings = product_stage_prescription_settings,
    .context = NULL,
  };
  return dajilog_system_core_register_implementation(&implementation);
}
