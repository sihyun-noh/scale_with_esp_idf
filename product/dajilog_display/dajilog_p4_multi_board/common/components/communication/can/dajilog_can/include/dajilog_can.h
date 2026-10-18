#pragma once
#include "dajilog_can_profile.h"
#include "freertos/FreeRTOS.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  uint32_t acceptance_code;
  uint32_t acceptance_mask;
  bool single_filter;
} dajilog_can_filter_t;

typedef struct {
  int tx_gpio;
  int rx_gpio;
  uint32_t bitrate;
  uint32_t tx_queue_length;
  uint32_t rx_queue_length;
  dajilog_can_filter_t filter;
} dajilog_can_config_t;
typedef enum {
  DAJILOG_CAN_STOPPED,
  DAJILOG_CAN_RUNNING,
  DAJILOG_CAN_BUS_OFF,
  DAJILOG_CAN_RECOVERING
} dajilog_can_state_t;
typedef struct {
  dajilog_can_state_t state;
  uint32_t pending_tx;
  uint32_t pending_rx;
  uint32_t tx_failed;
  uint32_t rx_missed;
  uint32_t bus_errors;
  uint32_t total_rx;
  uint32_t last_rx_id;
  uint8_t last_rx_dlc;
  bool last_rx_extended;
} dajilog_can_status_t;
esp_err_t dajilog_can_init(const dajilog_can_config_t *config);
esp_err_t dajilog_can_start(void);
esp_err_t dajilog_can_stop(void);
esp_err_t dajilog_can_deinit(void);
esp_err_t dajilog_can_write(const dajilog_can_frame_t *frame, TickType_t timeout);
esp_err_t dajilog_can_read(dajilog_can_frame_t *frame, TickType_t timeout);
esp_err_t dajilog_can_get_status(dajilog_can_status_t *status);
esp_err_t dajilog_can_recover(void);
dajilog_can_filter_t dajilog_can_filter_accept_all(void);
#ifdef __cplusplus
}
#endif
