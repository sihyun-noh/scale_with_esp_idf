
#include <stdio.h>
#include <stdint.h>
#include "mqtt_config.h"
#include "cJSON.h"
#include "esp_log.h"

static const char *TAG = "mqtt_handler";

typedef struct {
  char *name;
  char *mac;
  uint16_t sampling_period;
  uint16_t processing_period;
  uint16_t server_into_interval_period;
} sensor_info_t;

typedef struct {
} device_info_t;

sensor_info_t sensor_port[10] = { 0 };

void handle_json_payload(const char *payload) {
  cJSON *json = cJSON_Parse(payload);
  if (!json)
    return;

  const cJSON *sampling_period = cJSON_GetObjectItem(json, "sampling_period");
  const cJSON *processing_period = cJSON_GetObjectItem(json, "processing_period");
  const cJSON *server_into_interval_period = cJSON_GetObjectItem(json, "server_into_interval_period");
  const cJSON *device = cJSON_GetObjectItem(json, "device");
  const cJSON *status = cJSON_GetObjectItem(json, "status");
  if (cJSON_IsString(device) && cJSON_IsString(status)) {
    ESP_LOGI(TAG, "[JSON] Device: %s, Status: %s", device->valuestring, status->valuestring);
  }

  if (cJSON_IsString(sampling_period) && cJSON_IsString(processing_period) &&
      cJSON_IsString(server_into_interval_period)) {
    ESP_LOGI(TAG, "[JSON] sampling_period: %s, processing_period: %s, server_into_interval_period: %s",
             sampling_period->valuestring, processing_period->valuestring, server_into_interval_period->valuestring);
  }

  cJSON_Delete(json);
}

void handle_command_payload(const char *payload) {
  cJSON *json = cJSON_Parse(payload);
  if (!json)
    return;

  const cJSON *cmd = cJSON_GetObjectItem(json, "cmd");
  if (cJSON_IsString(cmd)) {
    ESP_LOGI(TAG, "[CMD] Received command: %s", cmd->valuestring);
  }
  cJSON_Delete(json);
}
