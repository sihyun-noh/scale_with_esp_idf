
#include "sensor_cfg_config.h"
#include "nvs_cfg.h"
#include "esp_log.h"
#include "nvs.h"
#include "cJSON.c"
#include "esp_mac.h"

#define DEVICE_ID_KEY    "device_id"
#define NETWORK_TYPE_KEY "network_type"
#define MAC_ADDR_LEN     18

static const char *TAG = "[sensor_cfg]";

static sensor_port_cfg_t g_sensor_cfgs[SENSOR_PORT_COUNT];
SemaphoreHandle_t cfg_mutex = NULL;
static bool initialized = false;

static sensor_cfg_manager_t sensor_cfg_mgr = { .cfg = g_sensor_cfgs, .mutex = NULL };

// singleton instance
sensor_port_cfg_t *sensor_cfg_instance(void) {
  return g_sensor_cfgs;
}

sensor_cfg_manager_t *sensor_cfg_get_instance(void) {
  // 뮤텍스가 아직 생성되지 않았다면 한 번만 생성
  if (!initialized) {
    sensor_cfg_mgr.mutex = xSemaphoreCreateMutex();
    if (sensor_cfg_mgr.mutex == NULL) {
      ESP_LOGE(TAG, "Failed to create sensor_cfg mutex");
      return NULL;
    }
    initialized = true;
    ESP_LOGI(TAG, "sensor_cfg_manager initialized");
  }
  return &sensor_cfg_mgr;
}

void sensor_cfg_lock(void) {
  sensor_cfg_manager_t *mgr = sensor_cfg_get_instance();
  if (mgr == NULL) {
    ESP_LOGE(TAG, "sensor_cfg_lock: manager not initialized");
    return;
  }
  xSemaphoreTake(mgr->mutex, portMAX_DELAY);
}

void sensor_cfg_unlock(void) {
  sensor_cfg_manager_t *mgr = sensor_cfg_get_instance();
  if (mgr == NULL) {
    ESP_LOGE(TAG, "sensor_cfg_unlock: manager not initialized");
    return;
  }
  xSemaphoreGive(mgr->mutex);
}

esp_err_t sensor_port_cfg_load(int port, sensor_port_cfg_t *cfg) {
  if (port < 1 || port > SENSOR_PORT_COUNT)
    return ESP_ERR_INVALID_ARG;

  char key[20];
  snprintf(key, sizeof(key), "sensor_port_%d", port);
  return cfg_get_blob(key, cfg, sizeof(sensor_port_cfg_t));
}

esp_err_t sensor_port_cfg_save(int port, const sensor_port_cfg_t *cfg) {
  if (port < 1 || port > SENSOR_PORT_COUNT)
    return ESP_ERR_INVALID_ARG;

  char key[20];
  snprintf(key, sizeof(key), "sensor_port_%d", port);
  return cfg_set_blob(key, cfg, sizeof(sensor_port_cfg_t));
}

sensor_port_cfg_t *sensor_port_cfg_get(int port) {
  if (port < 1 || port > SENSOR_PORT_COUNT)
    return NULL;
  ESP_LOGI(TAG, "get array buffer %d", port - 1);
  return &g_sensor_cfgs[port - 1];
}

esp_err_t sensor_port_cfg_init(void) {
  esp_err_t err = ESP_OK;

  // nvs 저장된 key 기반.
  for (int i = 1; i <= SENSOR_PORT_COUNT; ++i) {
    sensor_port_cfg_t *cfg = &g_sensor_cfgs[i - 1];
    cfg->port = i;

    err = sensor_port_cfg_load(i, cfg);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
      // 기본값 설정
      cfg->status.enabled = 1;
      strncpy(cfg->status.type_name, "UNKNOWN", sizeof(cfg->status.type_name));
      cfg->status.status_code = SENSOR_PORT_STATUS_NONE;

      cfg->sensor_type = SENSOR_TYPE_UNKNOWN;
      cfg->current_state = SENSOR_PORT_STATUS_NONE;

      // 서버 설정 기본값
      cfg->server_config.publish_interval = 60;
      cfg->server_config.threshold_enabled = 0;
      cfg->server_config.threshold_min = 0.0f;
      cfg->server_config.threshold_max = 100.0f;
      strncpy(cfg->dt_name, "UNSET", sizeof(cfg->dt_name));

      cfg->columns_size = 0;
      cfg->rows_size = 1;
      cfg->dirty = 1;  // 최초 저장 필요

      ESP_LOGI(TAG, "Default config set for port %d", i);
    } else if (err == ESP_OK) {
      cfg->dirty = 0;

      ESP_LOGI(TAG, "Loaded config for port %d", i);
      ESP_LOGI(TAG, "Port=%d | SensorType=%d | DT=%s | Enabled=%d | State=%d | Threshold=[%d, %.2f~%.2f]", cfg->port,
               cfg->sensor_type, cfg->dt_name, cfg->status.enabled, cfg->current_state,
               cfg->server_config.threshold_enabled, cfg->server_config.threshold_min,
               cfg->server_config.threshold_max);
    } else {
      ESP_LOGE(TAG, "Failed to load config for port %d: %s", i, esp_err_to_name(err));
    }
  }

  return ESP_OK;
}

esp_err_t sensor_port_cfg_commit(void) {
  esp_err_t err = ESP_OK;
  for (int i = 0; i < SENSOR_PORT_COUNT; ++i) {
    sensor_port_cfg_t *cfg = &g_sensor_cfgs[i];
    if (cfg->dirty) {
      err = sensor_port_cfg_save(i + 1, cfg);
      if (err == ESP_OK) {
        cfg->dirty = 0;
        ESP_LOGI(TAG, "Saved port %d config to NVS(cfg)", i + 1);
      } else {
        ESP_LOGE(TAG, "Failed to save config for port %d: %s", i + 1, esp_err_to_name(err));
      }
    }
  }
  return err;
}
#if 0
esp_err_t handle_mqtt_config_update(const char *json_payload) {
  if (!json_payload)
    return ESP_ERR_INVALID_ARG;

  cJSON *root = cJSON_Parse(json_payload);
  if (!root)
    return ESP_FAIL;

  cJSON *port = cJSON_GetObjectItem(root, "port");
  if (!port || !cJSON_IsNumber(port)) {
    ESP_LOGW(TAG, "Invalid or missing 'port'");
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  uint8_t new_port = port->valueint;
  if (new_port < 1 || new_port > SENSOR_PORT_COUNT) {
    ESP_LOGW(TAG, "Port number out of range: %d", new_port);
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  sensor_port_cfg_t *cfg = sensor_port_cfg_get(new_port);
  if (!cfg) {
    ESP_LOGE(TAG, "Failed to get config for port %d", new_port);
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  cJSON *enabled = cJSON_GetObjectItem(root, "enabled");
  cJSON *threshold = cJSON_GetObjectItem(root, "threshold");
  cJSON *sensor_type = cJSON_GetObjectItem(root, "sensor_type");

  if (enabled && cJSON_IsNumber(port)) {
    cfg->port = port->valueint;
    cfg->dirty = 1;
  }
  if (enabled && cJSON_IsNumber(enabled)) {
    cfg->enabled = enabled->valueint;
    cfg->dirty = 1;
  }
  if (threshold && cJSON_IsNumber(threshold)) {
    cfg->threshold = threshold->valueint;
    cfg->dirty = 1;
  }
  if (sensor_type && cJSON_IsNumber(sensor_type)) {
    cfg->sensor_type = sensor_type->valueint;
    cfg->dirty = 1;
  }

  cJSON_Delete(root);
  return ESP_OK;
}
#endif

esp_err_t handle_mqtt_config_update(const char *json_payload) {
  if (!json_payload)
    return ESP_ERR_INVALID_ARG;

  cJSON *root = cJSON_Parse(json_payload);
  if (!root) {
    ESP_LOGE(TAG, "Failed to parse JSON");
    return ESP_FAIL;
  }

  // Parse "port"
  cJSON *port = cJSON_GetObjectItem(root, "port");
  if (!port || !cJSON_IsNumber(port)) {
    ESP_LOGW(TAG, "Invalid or missing 'port'");
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  uint8_t new_port = port->valueint;
  if (new_port >= SENSOR_PORT_COUNT) {
    ESP_LOGW(TAG, "Port number out of range: %d", new_port);
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  sensor_port_cfg_t *cfg = sensor_port_cfg_get(new_port);
  if (!cfg) {
    ESP_LOGE(TAG, "Failed to get config for port %d", new_port);
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  // Parse "publish_interval"
  cJSON *interval = cJSON_GetObjectItem(root, "publish_interval");
  if (interval && cJSON_IsNumber(interval)) {
    cfg->server_config.publish_interval = interval->valueint;
    cfg->dirty = 1;
  }

  // Parse "threshold" object
  cJSON *threshold = cJSON_GetObjectItem(root, "threshold");
  if (threshold && cJSON_IsObject(threshold)) {
    cJSON *enabled = cJSON_GetObjectItem(threshold, "enabled");
    cJSON *min = cJSON_GetObjectItem(threshold, "min");
    cJSON *max = cJSON_GetObjectItem(threshold, "max");

    if (enabled && cJSON_IsBool(enabled)) {
      cfg->server_config.threshold_enabled = cJSON_IsTrue(enabled);
      cfg->dirty = 1;
    }
    if (min && cJSON_IsNumber(min)) {
      cfg->server_config.threshold_min = (float)min->valuedouble;
      cfg->dirty = 1;
    }
    if (max && cJSON_IsNumber(max)) {
      cfg->server_config.threshold_max = (float)max->valuedouble;
      cfg->dirty = 1;
    }
  }

  cJSON_Delete(root);
  return ESP_OK;
}

esp_err_t cfg_init_device_id(void) {
  char *existing = NULL;
  if (cfg_get_str(DEVICE_ID_KEY, &existing) == ESP_OK) {
    ESP_LOGI(TAG, "Device ID exists: %s", existing);
    free(existing);
    return ESP_OK;
  }

  uint8_t mac[6];
  esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
  if (err != ESP_OK) {
    err = esp_read_mac(mac, ESP_MAC_ETH);
  }
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read MAC");
    return err;
  }

  char mac_str[MAC_ADDR_LEN];
  snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  err = cfg_set_str(DEVICE_ID_KEY, mac_str);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Stored new device ID: %s", mac_str);
  }
  return err;
}

esp_err_t get_device_id(char **out_id) {
  if (!out_id)
    return ESP_ERR_INVALID_ARG;
  return cfg_get_str(DEVICE_ID_KEY, out_id);
}

esp_err_t cfg_set_network_type(const char *type_str) {
  if (!type_str)
    return ESP_ERR_INVALID_ARG;

  char *existing = NULL;
  esp_err_t err = cfg_get_str(NETWORK_TYPE_KEY, &existing);

  if (err == ESP_OK) {
    bool same = (strcmp(existing, type_str) == 0);
    free(existing);

    if (same) {
      ESP_LOGW(TAG, "Network type unchanged: %s", type_str);
      return ESP_OK;
    }
  } else if (err != ESP_ERR_NVS_NOT_FOUND) {
    // ESP_ERR_NVS_NOT_FOUND 외 다른 오류면.
    ESP_LOGE(TAG, "cfg_get_str failed: %s", esp_err_to_name(err));
    return err;
  }
  err = cfg_set_str(NETWORK_TYPE_KEY, type_str);
  if (err == ESP_OK) {
    ESP_LOGW(TAG, "Stored new network type: %s", type_str);
  } else {
    ESP_LOGE(TAG, "cfg_set_str failed: %s", esp_err_to_name(err));
  }
  return err;
}

esp_err_t cfg_get_network_type(char **out_type) {
  if (!out_type)
    return ESP_ERR_INVALID_ARG;
  return cfg_get_str(NETWORK_TYPE_KEY, out_type);
}
