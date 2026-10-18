#include "dajilog_can_device_manager.h"
#include <string.h>
#include "dajilog_can.h"
#include "dajilog_can_profiles_generated.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
static const char *TAG = "DajilogCANMgr";
static dajilog_can_device_state_t s_states[16];
static SemaphoreHandle_t s_lock;
static bool s_started;
static void rx_task(void *arg) {
  (void)arg;
  uint32_t raw_count = 0;
  for (;;) {
    dajilog_can_frame_t f;
    if (dajilog_can_read(&f, pdMS_TO_TICKS(100)) == ESP_OK) {
      raw_count++;
      if (raw_count <= 5U || (raw_count % 100U) == 0U) {
        ESP_LOGI(TAG, "raw RX #%lu: id=0x%03lX %s dlc=%u data=%02X %02X %02X %02X %02X %02X %02X %02X",
                 (unsigned long)raw_count, (unsigned long)f.identifier, f.extended ? "EXT" : "STD", (unsigned)f.dlc,
                 f.data[0], f.data[1], f.data[2], f.data[3], f.data[4], f.data[5], f.data[6], f.data[7]);
      }
      bool matched = false;
      for (size_t i = 0; i < DAJILOG_CAN_DEVICE_COUNT && i < 16; i++) {
        const dajilog_can_device_config_t *d = &DAJILOG_CAN_DEVICES[i];
        if (!d->enabled || d->status_id != f.identifier)
          continue;
        matched = true;
        const dajilog_can_protocol_profile_t *p = &DAJILOG_CAN_PROFILES[d->profile_index];
        uint8_t decoded[DAJILOG_CAN_MAX_DECODED_SIZE] = { 0 };
        esp_err_t e = p->decode_status(&f, decoded, sizeof(decoded));
        xSemaphoreTake(s_lock, portMAX_DELAY);
        s_states[i].last_error = e;
        if (e == ESP_OK) {
          memcpy(s_states[i].decoded, decoded, p->decoded_size);
          s_states[i].last_update_us = esp_timer_get_time();
          s_states[i].rx_count++;
          s_states[i].online = true;
        }
        xSemaphoreGive(s_lock);
        if (e != ESP_OK) {
          ESP_LOGW(TAG, "decode failed for %s: %s", d->instance_name, esp_err_to_name(e));
        }
      }
      if (!matched && raw_count <= 5U) {
        ESP_LOGW(TAG, "no device profile matched CAN ID 0x%03lX", (unsigned long)f.identifier);
      }
    }
    int64_t now = esp_timer_get_time();
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t i = 0; i < DAJILOG_CAN_DEVICE_COUNT && i < 16; i++) {
      if (s_states[i].online &&
          (now - s_states[i].last_update_us) > (int64_t)DAJILOG_CAN_DEVICES[i].timeout_ms * 1000LL)
        s_states[i].online = false;
    }
    xSemaphoreGive(s_lock);
  }
}
esp_err_t dajilog_can_device_manager_start(void) {
  if (s_started)
    return ESP_OK;
  if (DAJILOG_CAN_DEVICE_COUNT > 16)
    return ESP_ERR_NO_MEM;
  s_lock = xSemaphoreCreateMutex();
  if (!s_lock)
    return ESP_ERR_NO_MEM;
  if (xTaskCreate(rx_task, "can_device_rx", 4096, NULL, 10, NULL) != pdPASS) {
    vSemaphoreDelete(s_lock);
    s_lock = NULL;
    return ESP_ERR_NO_MEM;
  }
  s_started = true;
  ESP_LOGI(TAG, "device manager started with %u device(s)", (unsigned)DAJILOG_CAN_DEVICE_COUNT);
  return ESP_OK;
}
size_t dajilog_can_device_manager_count(void) {
  return DAJILOG_CAN_DEVICE_COUNT;
}
esp_err_t dajilog_can_device_manager_get(size_t i, const dajilog_can_device_config_t **c,
                                         dajilog_can_device_state_t *s) {
  if (!s_started || i >= DAJILOG_CAN_DEVICE_COUNT || !c || !s)
    return ESP_ERR_INVALID_ARG;
  xSemaphoreTake(s_lock, portMAX_DELAY);
  *c = &DAJILOG_CAN_DEVICES[i];
  *s = s_states[i];
  xSemaphoreGive(s_lock);
  return ESP_OK;
}
