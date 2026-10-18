#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
  uint32_t identifier;
  uint8_t dlc;
  uint8_t data[8];
  bool extended;
  bool remote;
} dajilog_can_frame_t;
typedef esp_err_t (*dajilog_can_encode_fn)(const void *, uint32_t, dajilog_can_frame_t *);
typedef esp_err_t (*dajilog_can_decode_fn)(const dajilog_can_frame_t *, void *, size_t);
typedef struct {
  const char *name;
  const char *display_name;
  uint32_t version;
  uint32_t bitrate;
  uint32_t default_command_id;
  uint32_t default_status_id;
  uint8_t command_dlc;
  uint8_t status_dlc;
  size_t decoded_size;
  dajilog_can_encode_fn encode_command;
  dajilog_can_decode_fn decode_status;
} dajilog_can_protocol_profile_t;
typedef struct {
  const char *instance_name;
  const char *display_name;
  size_t profile_index;
  uint32_t command_id;
  uint32_t status_id;
  uint32_t timeout_ms;
  bool enabled;
} dajilog_can_device_config_t;
#ifdef __cplusplus
}
#endif
