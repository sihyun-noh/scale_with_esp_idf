
#include "cJSON.h"
#include "esp_log.h"
#include "esp_system.h"
#include "mqtt_config.h"
#include "restart_reason.h"
#include "sensor_cfg_config.h"

static const char *TAG = "mqtt_publish";
extern mqtt_client_ctx_t mqtt_ctx;

static const char *esp_reset_reason_to_string(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "power_on";
    case ESP_RST_EXT: return "external_pin";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt_watchdog";
    case ESP_RST_TASK_WDT: return "task_watchdog";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep_sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unknown";
  }
}

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

  esp_reset_reason_t esp_reset_reason_id = esp_reset_reason();
  app_restart_reason_t restart_reason_id = APP_RESTART_REASON_NONE;
  esp_err_t restart_reason_err = restart_reason_load(&restart_reason_id);
  if (restart_reason_err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to load restart reason: %s", esp_err_to_name(restart_reason_err));
    restart_reason_id = APP_RESTART_REASON_UNKNOWN;
  }

  cJSON_AddNumberToObject(root, "espResetReasonId", esp_reset_reason_id);
  cJSON_AddStringToObject(root, "espResetReason", esp_reset_reason_to_string(esp_reset_reason_id));
  cJSON_AddNumberToObject(root, "restartReasonId", restart_reason_id);
  cJSON_AddStringToObject(root, "restartReason", restart_reason_to_string(restart_reason_id));

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
    int message_id = esp_mqtt_client_publish(client, topic, json_str, 0, 1, 0);
    if (message_id >= 0 && restart_reason_id != APP_RESTART_REASON_NONE &&
        restart_reason_id != APP_RESTART_REASON_UNKNOWN) {
      esp_err_t clear_err = restart_reason_clear();
      if (clear_err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to clear restart reason: %s", esp_err_to_name(clear_err));
      }
    }
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
  ESP_LOGW(TAG, "Publish sensor data to the server via MQTT.");
  /* Qos 0 at most once*/
  return esp_mqtt_client_publish(mqtt_ctx.client, topic, data_table, 0, 0, 0);
}

int publish_data(const char *topic, const char *json_data) {
  /* Qos 0 at most once*/
  return esp_mqtt_client_publish(mqtt_ctx.client, topic, json_data, 0, 0, 0);
}
