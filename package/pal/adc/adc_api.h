#ifndef _ADC_API_H_
#define _ADC_API_H_

#include "stddef.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initialize the given ADC bus
 *
 * The bus MUST not be acquired before initializing it, as this is handled
 * internally by the adc_init function!
 *
 */
bool adc_calibration_init(void);

/**
 * @brief   Convenience function for reading adc value from a device
 *
 * @param[in]  pin          ADC peripheral device
 *
 * @return                  0 When success otherwise negative
 */

int adc_read(uint8_t chan);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_API_H_ */
