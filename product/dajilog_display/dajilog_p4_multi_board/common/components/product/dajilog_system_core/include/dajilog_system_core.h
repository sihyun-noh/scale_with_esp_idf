#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dajilog_product_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    esp_err_t (*start)(void *context);
    esp_err_t (*command)(const dajilog_manual_motor_command_t *command,
                         void *context);
    esp_err_t (*stop)(void *context);
    bool (*is_running)(void *context);
    void *context;
} dajilog_motor_backend_t;

typedef struct {
    esp_err_t (*start)(void *context);
    bool (*is_started)(void *context);
    esp_err_t (*get_snapshot)(dajilog_dashboard_snapshot_t *snapshot,
                              void *context);
    esp_err_t (*update_vehicle)(float speed_kmh, bool online,
                                uint32_t signal_age_ms, void *context);
    esp_err_t (*update_motor)(size_t index,
                              const dajilog_motor_snapshot_t *motor,
                              void *context);
    esp_err_t (*register_motor_backend)(
        const dajilog_motor_backend_t *backend, void *context);
    void *context;
} dajilog_system_core_implementation_t;

esp_err_t dajilog_system_core_register_implementation(
    const dajilog_system_core_implementation_t *implementation);
esp_err_t dajilog_system_core_register_motor_backend(
    const dajilog_motor_backend_t *backend);
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
