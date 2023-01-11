#include "adc_api.h"
#include "esp32/adc_hal.h"

void adc_init(uint8_t channel) {
  adc_hal_init(channel);
  return;
}

int adc_read(uint8_t channel) {
  return adc_hal_read(channel);
}

int adc_read_voltage(uint8_t channel) {
  return adc_hal_read_voltage(channel);
}
