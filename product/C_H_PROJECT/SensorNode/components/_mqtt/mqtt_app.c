
#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "mqtt_config.h"
#include "mqtt_handler.h"
#include "mqtt_publish.h"
#include "sensor_cfg_config.h"
#include "sdkconfig.h"

static const char *TAG = "mqtt_app";

mqtt_client_ctx_t mqtt_ctx;
QueueHandle_t router_queue;

char topic[MAX_TOPIC_LEN] = { 0 };
char prefix_buff[MAX_TOPIC_LEN] = { 0 };

const char *topic_type_to_str(topic_type_t type) {
  switch (type) {
    case TOPIC_TYPE_STATE: return TOPIC_STATE;
    case TOPIC_TYPE_CONFIG: return TOPIC_CONFIG;
    case TOPIC_TYPE_SENSOR: return TOPIC_SENSOR;
    case TOPIC_TYPE_CMD: return TOPIC_CMD;
    case TOPIC_TYPE_JSON: return TOPIC_JSON;
    case TOPIC_TYPE_OTA_UPLOAD: return TOPIC_OTA_UPLOAD;
    case TOPIC_TYPE_SETTING: return TOPIC_SETTING;
    case TOPIC_TYPE_UPLOAD: return TOPIC_UPLOAD;
    default: return "unknown";
  }
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      ESP_LOGI(TAG, "MQTT connected");

      memset(prefix_buff, 0x00, sizeof(prefix_buff));
      BUILD_DEVICE_TOPIC(prefix_buff, TOPIC_TYPE_STATE);
      publish_device_status(client, "connected", prefix_buff);
      publish_command(client, "init");

      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_STATE) == ESP_OK) {
        esp_mqtt_client_subscribe(client, topic, 1);
      }
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_OTA_UPLOAD) == ESP_OK) {
        esp_mqtt_client_subscribe(client, topic, 1);
      }
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_UPLOAD) == ESP_OK) {
        esp_mqtt_client_subscribe(client, topic, 1);
      }
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_SETTING) == ESP_OK) {
        esp_mqtt_client_subscribe(client, topic, 1);
      }

      break;

    case MQTT_EVENT_DATA: {
      ESP_LOGI(TAG, "MQTT event data");
      mqtt_message_t msg = { 0 };
      memcpy(msg.topic, event->topic, event->topic_len);
      msg.topic[event->topic_len] = '\0';
      // strncpy(msg.payload, event->data, sizeof(msg.payload) - 1);
      memcpy(msg.payload, event->data, event->data_len);
      msg.payload[event->data_len] = '\0';
      xQueueSend(router_queue, &msg, 0);

      break;
    }

    default: break;
  }
}

esp_err_t build_device_topic_ext(topic_type_t type, char *buf, size_t len) {
  if (!buf)
    return ESP_ERR_INVALID_ARG;

  char *device_id = NULL;
  esp_err_t err = get_device_id(&device_id);
  if (err != ESP_OK)
    return err;

  snprintf(buf, len, "%s/%s/%s", TOPIC_PREFIX, device_id, topic_type_to_str(type));

  free(device_id);
  return ESP_OK;
}

void mqtt_init(void) {
  const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = CONFIG_MQTT_BROKER_URL,
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(client);
  mqtt_ctx.client = client;
}

// =============================
// 전략 등록 테이블
// =============================
/* static strategy_entry_t strategy_table[MAX_STRATEGIES]; */
/* static int strategy_count = 0; */

static strategy_manager_t strategy_instance = { 0 };  // Singleton instance

strategy_manager_t *strategy_manager_get_instance(void) {
  return &strategy_instance;
}

bool strategy_manager_register(const char *topic_prefix, const char *task_name, strategy_fn_t fn) {
  strategy_manager_t *inst = strategy_manager_get_instance();
  if (inst->count >= MAX_STRATEGIES)
    return false;

  strategy_entry_t *entry = &inst->table[inst->count];
  strncpy(entry->topic_prefix, topic_prefix, MAX_TOPIC_LEN);
  strncpy(entry->task_name, task_name, MAX_TASK_NAME_LEN);
  entry->strategy = fn;
  inst->count++;
  return true;
}

strategy_fn_t strategy_manager_find_strategy(const char *topic) {
  strategy_manager_t *inst = strategy_manager_get_instance();
  for (int i = 0; i < inst->count; ++i) {
    if (strstr(topic, inst->table[i].topic_prefix)) {
      return inst->table[i].strategy;
    }
  }
  return NULL;
}

const char *strategy_manager_get_task_name_for_topic(const char *topic) {
  strategy_manager_t *inst = strategy_manager_get_instance();
  for (int i = 0; i < inst->count; ++i) {
    if (strstr(topic, inst->table[i].topic_prefix)) {
      return inst->table[i].task_name;
    }
  }
  return NULL;
}

static topic_router_fn_t _queue_mapper = NULL;

void router_register_queue_mapper(topic_router_fn_t fn) {
  ESP_LOGW(TAG, "queue mapper!!");
  _queue_mapper = fn;
}

QueueHandle_t _get_task_queue_by_topic(const char *topic) {
  return _queue_mapper ? _queue_mapper(topic) : NULL;
}

// =============================
// RouterTask
// =============================

void router_dispatch_task(void *param) {
  router_queue = xQueueCreate(10, sizeof(mqtt_message_t));
  mqtt_message_t msg;
  while (1) {
    if (xQueueReceive(router_queue, &msg, portMAX_DELAY)) {
      ESP_LOGW(TAG, "receive topic: %s", msg.topic);
      QueueHandle_t target = _get_task_queue_by_topic(msg.topic);
      if (target) {
        xQueueSend(target, &msg, 0);
      } else {
        ESP_LOGW(TAG, "No task found for topic: %s", msg.topic);
      }
    }
  }
}
