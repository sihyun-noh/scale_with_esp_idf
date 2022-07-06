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
 * @return bool true on success, false on failure
 */
bool adc_calibration_init(void);

/**
 * @brief   Convenience function for reading adc value from a device
 *
 * @param[in]  pin          ADC peripheral device
 *
 * @return                  ADC value
 */
int adc_read(uint8_t chan);

/**
 * @brief   Convenience function for Convert an ADC reading to voltage in mV.
 *
 * @param[in]  pin          ADC peripheral device
 *
 * @return                  Voltage in mV
 */
int adc_read_to_voltage(uint8_t chan);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_API_H_ */
