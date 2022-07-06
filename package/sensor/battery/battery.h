/** @file battery.h
 *
 * @brief Device driver for BATTERY
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _BATTERY_H_
#define _BATTERY_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the underlying battery port
 *
 * @return bool true on success, false on failure
 */
bool battery_init(void);

/**
 * @brief   Convenience function for battery reading ADC
 *
 * @param[in]  chan          GPIO ADC port
 *
 * @return                   battery ADC value
 */
int read_battery(uint8_t chan);

/**
 * @brief   Convenience function for battery ADC reading to voltage in mV.
 *
 * @param[in]  chan          GPIO ADC port
 * @param[in]  ratio         Voltage ratio
 *
 * @return                   battery Voltage in mV
 */
int read_battery_voltage(uint8_t chan, float ratio);

/**
 * @brief   Convenience function for turn on battery gpio
 *
 * @param[in]  pin          GPIO peripheral device
 *
 * @return                  0 When success otherwise negative
 */
int battery_read_on(uint8_t pin);

/**
 * @brief   Convenience function for turn off battery gpio
 *
 * @param[in]  pin          GPIO peripheral device
 *
 * @return                  0 When success otherwise negative
 */
int battery_read_off(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif /* _BATTERY_H_ */
