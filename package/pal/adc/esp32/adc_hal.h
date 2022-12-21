/** @file adc_hal.h
 *
 * @brief ADC HAL api to support esp32 adc driver apis
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _ADC_HAL_H_
#define _ADC_HAL_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the underlying ESP32 adc calibration
 *
 * @return bool true on success, false on failure
 */
bool adc_hal_calibration_init(void);

/**
 * @brief   Convenience function for reading adc value ESP32
 *
 * @param[in]  pin          ADC peripheral device
 *
 * @return                  0 When success otherwise negative
 */
int adc_hal_read(uint8_t pin);

/**
 * @brief   Convenience function for Convert an ADC reading to voltage in mV in ESP32
 *
 * @param[in]  pin          ADC peripheral device
 *
 * @return                  Voltage in mV
 */
int adc_hal_read_to_voltage(uint8_t pin);

#endif /* _ADC_HAL_H_ */
