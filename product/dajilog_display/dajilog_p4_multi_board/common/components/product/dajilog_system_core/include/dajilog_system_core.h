#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dajilog_product_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t dajilog_system_core_start(void);
bool dajilog_system_core_is_started(void);
esp_err_t dajilog_system_core_get_snapshot(dajilog_dashboard_snapshot_t *snapshot);
esp_err_t dajilog_system_core_update_vehicle(float speed_kmh, bool online,
                                             uint32_t signal_age_ms);
esp_err_t dajilog_system_core_update_motor(size_t index,
                                           const dajilog_motor_snapshot_t *motor);

#ifdef __cplusplus
}
#endif
