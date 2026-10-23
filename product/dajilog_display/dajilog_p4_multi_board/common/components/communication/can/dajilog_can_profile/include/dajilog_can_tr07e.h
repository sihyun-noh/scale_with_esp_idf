#pragma once
#include "dajilog_can_profile.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  DAJILOG_TR07E_BIDIRECTIONAL = 0,
  DAJILOG_TR07E_FORWARD_ONLY = 1,
  DAJILOG_TR07E_REVERSE_ONLY = 2
} dajilog_tr07e_speed_direction_t;
typedef struct {
  uint8_t sensitivity, tx_power;
  dajilog_tr07e_speed_direction_t speed_direction;
  uint8_t power_limit, angle;
} dajilog_tr07e_config_t;
typedef struct {
  float speed_mps;
  uint16_t rcs_dbm;
  bool reverse;
} dajilog_tr07e_measurement_t;
/** @brief Encode a TR07E configuration payload into a CAN command frame. */
esp_err_t dajilog_tr07e_encode_config(const void *, uint32_t, dajilog_can_frame_t *);
/** @brief Decode a TR07E status frame into speed and direction measurements. */
esp_err_t dajilog_tr07e_decode_measurement(const dajilog_can_frame_t *, void *, size_t);
#ifdef __cplusplus
}
#endif
