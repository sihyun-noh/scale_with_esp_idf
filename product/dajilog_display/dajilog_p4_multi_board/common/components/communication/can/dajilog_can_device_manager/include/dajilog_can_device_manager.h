#pragma once
#include "dajilog_can_profile.h"
#include "freertos/FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
#define DAJILOG_CAN_MAX_DECODED_SIZE 64U
typedef struct {
  bool online;
  int64_t last_update_us;
  uint32_t rx_count;
  esp_err_t last_error;
  uint8_t decoded[DAJILOG_CAN_MAX_DECODED_SIZE];
} dajilog_can_device_state_t;

typedef void (*dajilog_can_device_observer_t)(
    size_t index,
    const dajilog_can_device_config_t *config,
    const dajilog_can_protocol_profile_t *profile,
    const dajilog_can_device_state_t *state,
    void *context);

esp_err_t dajilog_can_device_manager_set_observer(
    dajilog_can_device_observer_t observer, void *context);
esp_err_t dajilog_can_device_manager_add_observer(
    dajilog_can_device_observer_t observer, void *context);
esp_err_t dajilog_can_device_manager_start(void);
esp_err_t dajilog_can_device_manager_get(size_t index, const dajilog_can_device_config_t **config,
                                         dajilog_can_device_state_t *state);
esp_err_t dajilog_can_device_manager_send(size_t index, const void *command,
                                          TickType_t timeout);
size_t dajilog_can_device_manager_count(void);
#ifdef __cplusplus
}
#endif
