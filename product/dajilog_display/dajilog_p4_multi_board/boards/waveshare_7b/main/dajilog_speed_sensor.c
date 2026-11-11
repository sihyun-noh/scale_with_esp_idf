#include "dajilog_speed_sensor.h"

#include <math.h>
#include <string.h>

#include "dajilog_can_device_manager.h"
#include "dajilog_can_tr07e.h"
#include "dajilog_system_core.h"
#include "esp_log.h"
#include "esp_timer.h"
#ifdef CONFIG_DAJILOG_SPEED_SIMULATION
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

static const char* TAG = "DajilogSpeed7B";

#ifdef CONFIG_DAJILOG_SPEED_SIMULATION
/** @brief [시험 전용 속도] TR07E 없이 1→4→2 km/h를 반복해 제품 Queue에 보낸다. */
static void speed_simulation_task(void* context) {
  (void)context;
  static const float speeds_kmh[] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 3.0f, 2.0f };
  size_t step = 0;
  uint8_t samples_at_step = 0;
  ESP_LOGW(TAG, "TR07E speed simulation enabled; real TR07E frames are ignored");
  for (;;) {
    if (dajilog_system_core_is_started()) {
      if (samples_at_step == 0) {
        ESP_LOGI(TAG, "SIM speed %.1f km/h", (double)speeds_kmh[step]);
      }
      const esp_err_t error = dajilog_system_core_update_vehicle(speeds_kmh[step], true, false, 0U);
      if (error != ESP_OK) {
        ESP_LOGW(TAG, "SIM speed update dropped: %s", esp_err_to_name(error));
      }
      /* AUTO의 0.5V ramp가 각 속도에서 반응할 시간을 확보한다. */
      if (++samples_at_step == 25U) {
        samples_at_step = 0;
        step = (step + 1U) % (sizeof(speeds_kmh) / sizeof(speeds_kmh[0]));
      }
    }
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
#endif

#ifndef CONFIG_DAJILOG_SPEED_SIMULATION
/**
 * @brief Convert a decoded TR07E measurement to km/h and queue it to Core.
 *
 * This callback runs from CAN Device Manager notification context and performs
 * no LVGL operations.
 */
static void speed_sensor_observer(size_t index, const dajilog_can_device_config_t* config,
                                  const dajilog_can_protocol_profile_t* profile,
                                  const dajilog_can_device_state_t* state, void* context) {
  (void)index;
  (void)config;
  (void)context;
  if (profile == NULL || state == NULL || strcmp(profile->name, "tr07e") != 0) {
    return;
  }

  const dajilog_tr07e_measurement_t* measurement = (const dajilog_tr07e_measurement_t*)state->decoded;
  uint32_t signal_age_ms = 0;
  if (state->last_update_us > 0) {
    const int64_t age_us = esp_timer_get_time() - state->last_update_us;
    signal_age_ms = age_us > 0 ? (uint32_t)(age_us / 1000LL) : 0;
  }
  const float speed_kmh = state->online ? fabsf(measurement->speed_mps) * 3.6f : 0.0f;
  const esp_err_t error = dajilog_system_core_update_vehicle(speed_kmh, state->online,
                                                             state->online && measurement->reverse, signal_age_ms);
  if (error != ESP_OK) {
    ESP_LOGW(TAG, "TR07E update dropped: %s", esp_err_to_name(error));
  }
}
#endif

/** @brief Subscribe the product speed adapter to CAN Device Manager updates. */
esp_err_t dajilog_speed_sensor_register(void) {
#ifdef CONFIG_DAJILOG_SPEED_SIMULATION
  return xTaskCreate(speed_simulation_task, "speed_sim_7b", 3072, NULL, 6, NULL) == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
#else
  return dajilog_can_device_manager_add_observer(speed_sensor_observer, NULL);
#endif
}
