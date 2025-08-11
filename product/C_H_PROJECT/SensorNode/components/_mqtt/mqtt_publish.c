
#include "cJSON.h"
#include "esp_log.h"
#include "mqtt_config.h"
#include "sensor_cfg_config.h"

static const char *TAG = "mqtt_publish";
extern mqtt_client_ctx_t mqtt_ctx;

void publish_device_status(esp_mqtt_client_handle_t client, const char *status, const char *topic) {
  char *device_id = NULL;
  char *network_type = NULL;
  get_device_id(&device_id);  // Must be freed later if heap allocated
  cfg_get_network_type(&network_type);

  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "deviceId", device_id);
  cJSON_AddStringToObject(root, "status", status);
  cJSON_AddStringToObject(root, "fw_ver", FW_VERSION);
  cJSON_AddStringToObject(root, "networkType", network_type);

  sensor_port_cfg_t *cfg = sensor_cfg_instance();
  if (!cfg) {
    ESP_LOGE(TAG, "sensor_cfg_instance() returned NULL");
    cJSON_Delete(root);
    return;
  }

  cJSON *ports_array = cJSON_CreateArray();
  for (int i = 0; i < SENSOR_PORT_COUNT; i++) {
    cJSON *port_obj = cJSON_CreateObject();

    cJSON_AddNumberToObject(port_obj, "port", cfg[i].port);
    cJSON_AddStringToObject(port_obj, "type", sensor_type_to_str(cfg[i].sensor_type));

    const char *status_str = NULL;
    switch (cfg[i].current_state) {
      case SENSOR_PORT_STATUS_NONE: status_str = "Disabled"; break;
      case SENSOR_PORT_STATUS_READY: status_str = "Ready"; break;
      case SENSOR_PORT_STATUS_ERROR: status_str = "error"; break;
      default: status_str = "unknown"; break;
    }

    cJSON_AddStringToObject(port_obj, "status", status_str);
    cJSON_AddStringToObject(port_obj, "dt_name", cfg[i].dt_name);

    cJSON_AddItemToArray(ports_array, port_obj);
  }

  cJSON_AddItemToObject(root, "ports", ports_array);

  char *json_str = cJSON_PrintUnformatted(root);
  if (json_str) {
    esp_mqtt_client_publish(client, topic, json_str, 0, 1, 0);
    free(json_str);
  }

  cJSON_Delete(root);
  if (device_id)
    free(device_id);

  if (network_type)
    free(network_type);
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

int publish_data(const char *topic, const char *json_data) {
  /* Qos 0 at most once*/
  return esp_mqtt_client_publish(mqtt_ctx.client, topic, json_data, 0, 0, 0);
}
