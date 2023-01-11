#ifndef _ADC_HAL_H_
#define _ADC_HAL_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

bool adc_hal_init(uint8_t channel);

int adc_hal_read(uint8_t channel);

int adc_hal_read_voltage(uint8_t channel);

#endif /* _ADC_HAL_H_ */
