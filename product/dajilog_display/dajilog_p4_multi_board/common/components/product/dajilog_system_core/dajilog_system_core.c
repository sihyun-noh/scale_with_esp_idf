#include "dajilog_system_core.h"

#include <string.h>

static dajilog_system_core_implementation_t s_implementation;
static bool s_registered;

esp_err_t dajilog_system_core_register_implementation(
    const dajilog_system_core_implementation_t *implementation)
{
    if (implementation == NULL || implementation->start == NULL ||
        implementation->is_started == NULL ||
        implementation->get_snapshot == NULL ||
        implementation->update_vehicle == NULL ||
        implementation->update_motor == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_registered && s_implementation.is_started(s_implementation.context)) {
        return ESP_ERR_INVALID_STATE;
    }
    s_implementation = *implementation;
    s_registered = true;
    return ESP_OK;
}

esp_err_t dajilog_system_core_register_motor_backend(
    const dajilog_motor_backend_t *backend)
{
    if (!s_registered || s_implementation.register_motor_backend == NULL) {
        return ESP_ERR_NOT_SUPPORTED;
    }
    return s_implementation.register_motor_backend(
        backend, s_implementation.context);
}

esp_err_t dajilog_system_core_start(void)
{
    return s_registered
               ? s_implementation.start(s_implementation.context)
               : ESP_ERR_NOT_SUPPORTED;
}

bool dajilog_system_core_is_started(void)
{
    return s_registered &&
           s_implementation.is_started(s_implementation.context);
}

esp_err_t dajilog_system_core_get_snapshot(
    dajilog_dashboard_snapshot_t *snapshot)
{
    return s_registered
               ? s_implementation.get_snapshot(snapshot,
                                                s_implementation.context)
               : ESP_ERR_NOT_SUPPORTED;
}

esp_err_t dajilog_system_core_update_vehicle(
    float speed_kmh, bool online, uint32_t signal_age_ms)
{
    return s_registered
               ? s_implementation.update_vehicle(
                     speed_kmh, online, signal_age_ms,
                     s_implementation.context)
               : ESP_ERR_NOT_SUPPORTED;
}

esp_err_t dajilog_system_core_update_motor(
    size_t index, const dajilog_motor_snapshot_t *motor)
{
    return s_registered
               ? s_implementation.update_motor(index, motor,
                                                s_implementation.context)
               : ESP_ERR_NOT_SUPPORTED;
}
