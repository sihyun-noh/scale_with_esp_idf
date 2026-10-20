#include "dajilog_can_moonwalker.h"

#include <string.h>

static bool valid_type(uint8_t type) {
  return type == DAJILOG_MOONWALKER_INT8 || type == DAJILOG_MOONWALKER_INT16 ||
         type == DAJILOG_MOONWALKER_INT32 || type == DAJILOG_MOONWALKER_FLOAT32;
}

static uint32_t read_le32(const uint8_t *data) {
  return (uint32_t)data[0] | ((uint32_t)data[1] << 8U) |
         ((uint32_t)data[2] << 16U) | ((uint32_t)data[3] << 24U);
}

static void write_le32(uint8_t *data, uint32_t value) {
  data[0] = (uint8_t)value;
  data[1] = (uint8_t)(value >> 8U);
  data[2] = (uint8_t)(value >> 16U);
  data[3] = (uint8_t)(value >> 24U);
}

esp_err_t dajilog_moonwalker_encode_request(const void *input, uint32_t id,
                                             dajilog_can_frame_t *frame) {
  const dajilog_moonwalker_request_t *request = input;
  if (request == NULL || frame == NULL || id > 0x7FFU ||
      (request->access != DAJILOG_MOONWALKER_READ &&
       request->access != DAJILOG_MOONWALKER_WRITE) ||
      !valid_type((uint8_t)request->value_type)) {
    return ESP_ERR_INVALID_ARG;
  }

  memset(frame, 0, sizeof(*frame));
  frame->identifier = id;
  frame->dlc = 8;
  frame->data[0] = (uint8_t)request->access | (uint8_t)request->value_type;
  frame->data[1] = (uint8_t)request->index;
  frame->data[2] = (uint8_t)(request->index >> 8U);
  frame->data[3] = request->sub_index;

  if (request->access == DAJILOG_MOONWALKER_WRITE) {
    uint32_t raw = 0;
    if (request->value_type == DAJILOG_MOONWALKER_FLOAT32) {
      memcpy(&raw, &request->value.f32, sizeof(raw));
    } else {
      raw = (uint32_t)request->value.i32;
    }
    write_le32(&frame->data[4], raw);
  }
  return ESP_OK;
}

esp_err_t dajilog_moonwalker_decode_response(const dajilog_can_frame_t *frame,
                                             void *output, size_t output_size) {
  if (frame == NULL || output == NULL ||
      output_size < sizeof(dajilog_moonwalker_response_t) || frame->extended ||
      frame->remote || frame->dlc < 2U) {
    return ESP_ERR_INVALID_ARG;
  }

  dajilog_moonwalker_response_t *response = output;
  memset(response, 0, sizeof(*response));
  if (frame->data[0] == 0x80U) {
    response->error_code = frame->data[1];
    return ESP_OK;
  }

  const uint8_t access = frame->data[0] & 0xF0U;
  const uint8_t type = frame->data[0] & 0x0FU;
  if ((access != 0x20U && access != 0x40U) || !valid_type(type) ||
      frame->dlc < 8U) {
    return ESP_ERR_INVALID_RESPONSE;
  }

  response->success = true;
  response->is_read_response = access == 0x40U;
  response->value_type = (dajilog_moonwalker_value_type_t)type;
  response->index = (uint16_t)frame->data[1] | ((uint16_t)frame->data[2] << 8U);
  response->sub_index = frame->data[3];
  const uint32_t raw = read_le32(&frame->data[4]);
  if (response->value_type == DAJILOG_MOONWALKER_FLOAT32) {
    memcpy(&response->value.f32, &raw, sizeof(raw));
  } else {
    response->value.i32 = (int32_t)raw;
  }
  return ESP_OK;
}
