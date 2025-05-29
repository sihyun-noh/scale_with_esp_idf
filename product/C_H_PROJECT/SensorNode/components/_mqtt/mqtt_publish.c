
#include "cJSON.h"
#include "mqtt_config.h"

static const char *TAG = "mqtt_publish";
extern mqtt_client_ctx_t mqtt_ctx;

void publish_device_status(esp_mqtt_client_handle_t client, const char *status) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "device", "esp32");
  cJSON_AddStringToObject(root, "status", status);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str) {
    esp_mqtt_client_publish(client, TOPIC_JSON, json_str, 0, 1, 0);
    free(json_str);
  }
  cJSON_Delete(root);
}

void publish_command(esp_mqtt_client_handle_t client, const char *cmd) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "cmd", cmd);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str) {
    esp_mqtt_client_publish(client, TOPIC_CMD, json_str, 0, 1, 0);
    free(json_str);
  }
  cJSON_Delete(root);
}

int publish_sensor_datatable(const char *topic, const char *data_table) {
  /* Qos 0 at most once*/
  return esp_mqtt_client_publish(mqtt_ctx.client, topic, data_table, 0, 0, 0);
}
