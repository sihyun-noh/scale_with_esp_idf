#include "dajilog_speed_sensor.h"

#include <math.h>
#include <string.h>

#include "dajilog_can_device_manager.h"
#include "dajilog_can_tr07e.h"
#include "dajilog_system_core.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "DajilogSpeed7B";

static void speed_sensor_observer(
    size_t index, const dajilog_can_device_config_t *config,
    const dajilog_can_protocol_profile_t *profile,
    const dajilog_can_device_state_t *state, void *context)
{
    (void)index;
    (void)config;
    (void)context;
    if (profile == NULL || state == NULL ||
        strcmp(profile->name, "tr07e") != 0) {
        return;
    }

    const dajilog_tr07e_measurement_t *measurement =
        (const dajilog_tr07e_measurement_t *)state->decoded;
    uint32_t signal_age_ms = 0;
    if (state->last_update_us > 0) {
        const int64_t age_us = esp_timer_get_time() - state->last_update_us;
        signal_age_ms = age_us > 0 ? (uint32_t)(age_us / 1000LL) : 0;
    }
    const float speed_kmh =
        state->online ? fabsf(measurement->speed_mps) * 3.6f : 0.0f;
    const esp_err_t error = dajilog_system_core_update_vehicle(
        speed_kmh, state->online, signal_age_ms);
    if (error != ESP_OK) {
        ESP_LOGW(TAG, "TR07E update dropped: %s", esp_err_to_name(error));
    }
}

esp_err_t dajilog_speed_sensor_register(void)
{
    return dajilog_can_device_manager_add_observer(
        speed_sensor_observer, NULL);
}
