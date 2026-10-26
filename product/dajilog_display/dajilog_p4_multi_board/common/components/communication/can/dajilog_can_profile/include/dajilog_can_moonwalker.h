#pragma once

#include "dajilog_can_profile.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DAJILOG_MOONWALKER_INT8 = 0x00,
  DAJILOG_MOONWALKER_INT16 = 0x04,
  DAJILOG_MOONWALKER_INT32 = 0x08,
  DAJILOG_MOONWALKER_FLOAT32 = 0x0C,
} dajilog_moonwalker_value_type_t;

typedef enum {
  DAJILOG_MOONWALKER_READ = 0x30,
  DAJILOG_MOONWALKER_WRITE = 0x10,
} dajilog_moonwalker_access_t;

typedef struct {
  dajilog_moonwalker_access_t access;
  dajilog_moonwalker_value_type_t value_type;
  uint16_t index;
  uint8_t sub_index;
  union {
    int32_t i32;
    float f32;
  } value;
} dajilog_moonwalker_request_t;

typedef struct {
  bool success;
  bool is_read_response;
  dajilog_moonwalker_value_type_t value_type;
  uint16_t index;
  uint8_t sub_index;
  uint8_t error_code;
  union {
    int32_t i32;
    float f32;
  } value;
} dajilog_moonwalker_response_t;

enum {
  DAJILOG_MOONWALKER_INDEX_USER_DATA = 0x0038,
  DAJILOG_MOONWALKER_INDEX_STATUS = 0x0066,
  DAJILOG_MOONWALKER_INDEX_FAULT = 0x0067,
  DAJILOG_MOONWALKER_INDEX_OUTPUT_COMMAND = 0x0072,
  DAJILOG_MOONWALKER_INDEX_TEMPERATURE = 0x0079,
  DAJILOG_MOONWALKER_INDEX_VOLTAGE = 0x007A,
  DAJILOG_MOONWALKER_INDEX_CURRENT = 0x007B,
  DAJILOG_MOONWALKER_INDEX_VELOCITY = 0x007C,
};

/** @brief Encode a typed MoonWalker read/write request into a CAN frame. */
esp_err_t dajilog_moonwalker_encode_request(const void *input, uint32_t id,
                                             dajilog_can_frame_t *frame);
/** @brief Decode a MoonWalker response frame into a typed response record. */
esp_err_t dajilog_moonwalker_decode_response(const dajilog_can_frame_t *frame,
                                             void *output, size_t output_size);

#ifdef __cplusplus
}
#endif
