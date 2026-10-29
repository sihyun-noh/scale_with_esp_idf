#include "restart_reason.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"

#define RESTART_REASON_NAMESPACE "system_state"
#define RESTART_REASON_KEY       "restart_reason"

static const char *TAG = "restart_reason";

esp_err_t restart_reason_save(app_restart_reason_t reason) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(RESTART_REASON_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    return err;
  }

  err = nvs_set_u8(handle, RESTART_REASON_KEY, (uint8_t)reason);
  if (err == ESP_OK) {
    err = nvs_commit(handle);
  }
  nvs_close(handle);
  return err;
}

esp_err_t restart_reason_load(app_restart_reason_t *reason) {
  if (!reason) {
    return ESP_ERR_INVALID_ARG;
  }

  *reason = APP_RESTART_REASON_NONE;

  nvs_handle_t handle;
  esp_err_t err = nvs_open(RESTART_REASON_NAMESPACE, NVS_READONLY, &handle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    return ESP_OK;
  }
  if (err != ESP_OK) {
    return err;
  }

  uint8_t stored_reason = APP_RESTART_REASON_NONE;
  err = nvs_get_u8(handle, RESTART_REASON_KEY, &stored_reason);
  nvs_close(handle);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
    return ESP_OK;
  }
  if (err != ESP_OK) {
    return err;
  }

  *reason = (app_restart_reason_t)stored_reason;
  return ESP_OK;
}

esp_err_t restart_reason_clear(void) {
  return restart_reason_save(APP_RESTART_REASON_NONE);
}

const char *restart_reason_to_string(app_restart_reason_t reason) {
  switch (reason) {
    case APP_RESTART_REASON_NONE: return "none";
    case APP_RESTART_REASON_SENSOR_TASK_TIMEOUT: return "sensor_task_timeout";
    case APP_RESTART_REASON_REMOTE_COMMAND: return "remote_command";
    case APP_RESTART_REASON_OTA_UPDATE: return "ota_update";
    case APP_RESTART_REASON_SENSOR_CONFIG_UPDATED: return "sensor_config_updated";
    case APP_RESTART_REASON_SENSOR_DETECTION_FAILED: return "sensor_detection_failed";
    case APP_RESTART_REASON_WIFI_CONFIG_UPDATED: return "wifi_config_updated";
    case APP_RESTART_REASON_SMARTCONFIG_UPDATED: return "smartconfig_updated";
    default: return "unknown";
  }
}

void restart_with_reason(app_restart_reason_t reason) {
  esp_err_t err = restart_reason_save(reason);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to save restart reason %u: %s", (unsigned int)reason, esp_err_to_name(err));
  } else {
    ESP_LOGW(TAG, "Restarting with reason: %s (%u)", restart_reason_to_string(reason), (unsigned int)reason);
  }

  esp_restart();
  __builtin_unreachable();
}
