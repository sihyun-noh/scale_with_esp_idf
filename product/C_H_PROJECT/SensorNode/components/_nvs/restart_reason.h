#ifndef RESTART_REASON_H
#define RESTART_REASON_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  APP_RESTART_REASON_NONE = 0,
  APP_RESTART_REASON_SENSOR_TASK_TIMEOUT,
  APP_RESTART_REASON_REMOTE_COMMAND,
  APP_RESTART_REASON_OTA_UPDATE,
  APP_RESTART_REASON_SENSOR_CONFIG_UPDATED,
  APP_RESTART_REASON_SENSOR_DETECTION_FAILED,
  APP_RESTART_REASON_WIFI_CONFIG_UPDATED,
  APP_RESTART_REASON_SMARTCONFIG_UPDATED,
  APP_RESTART_REASON_UNKNOWN = 255,
} app_restart_reason_t;

esp_err_t restart_reason_save(app_restart_reason_t reason);
esp_err_t restart_reason_load(app_restart_reason_t *reason);
esp_err_t restart_reason_clear(void);
const char *restart_reason_to_string(app_restart_reason_t reason);
void restart_with_reason(app_restart_reason_t reason) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif
