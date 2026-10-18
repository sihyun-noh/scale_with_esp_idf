#include "dajilog_can_tr07e.h"
#include <string.h>
esp_err_t dajilog_tr07e_encode_config(const void *input, uint32_t id, dajilog_can_frame_t *frame) {
  const dajilog_tr07e_config_t *c = input;
  if (!c || !frame || id > 0x7ffU || c->sensitivity < 1U || c->tx_power < 1U || c->tx_power > 16U ||
      c->speed_direction > 2 || c->power_limit < 1U || c->angle < 1U || c->angle > 80U)
    return ESP_ERR_INVALID_ARG;
  memset(frame, 0, sizeof(*frame));
  frame->identifier = id;
  frame->dlc = 5;
  frame->data[0] = c->sensitivity;
  frame->data[1] = c->tx_power;
  frame->data[2] = (uint8_t)c->speed_direction;
  frame->data[3] = c->power_limit;
  frame->data[4] = c->angle;
  return ESP_OK;
}
esp_err_t dajilog_tr07e_decode_measurement(const dajilog_can_frame_t *f, void *output, size_t size) {
  if (!f || !output || size < sizeof(dajilog_tr07e_measurement_t) || f->dlc < 4U || f->extended || f->remote)
    return ESP_ERR_INVALID_ARG;
  uint16_t speed = ((uint16_t)f->data[0] << 4U) | ((uint16_t)f->data[1] >> 4U);
  uint16_t rcs = (((uint16_t)f->data[1] & 0x0fU) << 8U) | f->data[2];
  dajilog_tr07e_measurement_t *m = output;
  m->speed_mps = (float)speed * 0.01f;
  m->rcs_dbm = rcs;
  m->reverse = (f->data[3] & 1U) != 0U;
  return ESP_OK;
}
