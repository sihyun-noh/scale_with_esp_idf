
// ===========================

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#include "mqtt_config.h"
#include "mqtt_handler.h"
#include "ota_update.h"
#include "sensor_auto_detect.h"
#include "sensor_cfg_config.h"

#define MAX_TASKS        10
#define MAC_ADDR_MAX_LEN 32

static const char *TAG = "TaskManager";

extern void dt_1min_smp_task(void *pvParameters);
extern void upload_file_multipart(const char *filepath);
extern esp_err_t file_upload_proseece(void);

// =============================
// 구조체 정의
// =============================

typedef struct {
  TaskHandle_t handle;
  QueueHandle_t queue;
  char name[16];
} managed_task_t;

typedef struct {
  int port;                    ///< port number (1~6)
  char mac[MAC_ADDR_MAX_LEN];  ///< MAC address string
} topic_info_t;

// =============================
// TaskManager
// =============================
static managed_task_t tasks[MAX_TASKS];
static int task_count = 0;

bool strategy_create_task(const char *name, uint32_t stack_size, TaskFunction_t fn, UBaseType_t priority) {
  if (task_count >= MAX_TASKS)
    return false;
  QueueHandle_t queue = xQueueCreate(10, sizeof(mqtt_message_t));
  xTaskCreatePinnedToCore(fn, name, stack_size, (void *)queue, priority, &tasks[task_count].handle, 1);
  tasks[task_count].queue = queue;
  strncpy(tasks[task_count].name, name, sizeof(tasks[task_count].name));
  task_count++;
  return true;
}

QueueHandle_t get_task_queue_by_topic(const char *topic) {
  const char *target_name = strategy_manager_get_task_name_for_topic(topic);
  if (!target_name) {
    ESP_LOGW(TAG, "No task mapped to topic: %s", topic);
    return NULL;
  }
  for (int i = 0; i < task_count; ++i) {
    if (strcmp(tasks[i].name, target_name) == 0) {
      ESP_LOGI(TAG, "Routing topic '%s' to task '%s'", topic, target_name);
      return tasks[i].queue;
    }
  }

  ESP_LOGW(TAG, "No task found for topic: %s", topic);
  return NULL;
}

esp_err_t parse_topic_info(const char *topic, topic_info_t *info) {
  if (!topic || !info)
    return ESP_FAIL;

  // 기대 형식: v1/device/{mac}/port/{n}/config
  const char *p = strstr(topic, "v1/device/");
  if (!p)
    return ESP_FAIL;

  p += strlen("v1/device/");  // MAC 시작 위치
  const char *mac_end = strchr(p, '/');
  if (!mac_end || mac_end - p >= MAC_ADDR_MAX_LEN)
    return ESP_FAIL;

  strncpy(info->mac, p, mac_end - p);
  info->mac[mac_end - p] = '\0';

  /* // 이제 /port/{n}/ 찾기 */
  /* const char *port_ptr = strstr(mac_end, "/port/"); */
  /* if (!port_ptr) */
  /*   return ESP_FAIL; */
  /**/
  /* port_ptr += strlen("/port/"); */
  /* int port = atoi(port_ptr); */
  /* if (port < 1 || port > SENSOR_PORT_COUNT) */
  /*   return ESP_FAIL; */
  /**/
  /* info->port = port; */
  return ESP_OK;
}

void strategy_setting_from_mqtt(const mqtt_message_t *msg) {
  ESP_LOGI(TAG, "[MQTT] Received configuration message");
  topic_info_t info;
  if (parse_topic_info(msg->topic, &info) == ESP_OK) {
    ESP_LOGI(TAG, "[MQTT] Parsed device MAC: %s", info.mac);
    handle_mqtt_config_update(msg->payload);
    sensor_port_cfg_commit();
  } else {
    ESP_LOGW(TAG, "[MQTT] Failed to parse topic: %s", msg->topic);
  }
}

void strategy_ota_start(const mqtt_message_t *msg) {
  ESP_LOGI(TAG, "[OTA] Received OTA trigger message");
  ESP_LOGI(TAG, "[OTA] Payload: %s", msg->payload);
  ota_start();
}

void strategy_upload_file_start(const mqtt_message_t *msg) {
  ESP_LOGI(TAG, "[Upload] Received file upload request");
  ESP_LOGI(TAG, "[Upload] Payload: %s", msg->payload);
  file_upload_proseece();
}

// =============================
// Worker Tasks
// =============================
void strategy_task(void *param) {
  QueueHandle_t queue = (QueueHandle_t)param;
  TaskHandle_t task = xTaskGetCurrentTaskHandle();
  mqtt_message_t msg;
  ESP_LOGW(TAG, "Task name :%s", pcTaskGetName(task));
  while (1) {
    if (xQueueReceive(queue, &msg, portMAX_DELAY)) {
      strategy_fn_t fn = strategy_manager_find_strategy(msg.topic);
      if (fn)
        fn(&msg);
    }
  }
}
esp_err_t strategy_register_topic(topic_type_t type, const char *task_name, strategy_fn_t fn) {
  char topic_prefix[MAX_TOPIC_LEN] = { 0 };

  if (BUILD_DEVICE_TOPIC(topic_prefix, type) == ESP_OK) {
    ESP_LOGW(TAG, "Task topic_prefix : %s", topic_prefix);
    if (!strategy_manager_register(topic_prefix, task_name, fn)) {
      return ESP_FAIL;
    }
  }
  return ESP_OK;
}

// =============================
// app_main
// =============================
void app_main_task(void) {
  // task 4개 만들면 overflow
  // mqtt에 의해서만 동작하는 task 들만strategy로
  strategy_register_topic(TOPIC_TYPE_SETTING, "setting_task", strategy_setting_from_mqtt);
  strategy_register_topic(TOPIC_TYPE_OTA, "ota_task", strategy_ota_start);
  strategy_register_topic(TOPIC_TYPE_UPLOAD, "upload_file_task", strategy_upload_file_start);

  strategy_create_task("setting_task", 4096, strategy_task, 9);
  strategy_create_task("ota_task", 4096, strategy_task, 9);
  strategy_create_task("upload_file_task", 8192, strategy_task, 9);

  xTaskCreate(router_task, "router_task", 4096, NULL, 10, NULL);
  // Callback function register
  router_register_queue_mapper(get_task_queue_by_topic);

  xTaskCreate(dt_1min_smp_task, "sensor_logger_task", 4096, NULL, 5, NULL);
  // xTaskCreate(sensor_auto_detect_task, "sensor_auto_detect_task", 4096, NULL, 10, NULL);
}
