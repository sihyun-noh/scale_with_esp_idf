#ifndef _ADC_API_H_
#define _ADC_API_H_

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

void adc_init(uint8_t channel);

int adc_read(uint8_t channel);

int adc_read_voltage(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_API_H_ */
