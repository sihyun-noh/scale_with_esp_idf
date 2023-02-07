#ifndef _SCD4X_H_
#define _SCD4X_H_

/** @file scd4x.h
 *
 * @brief Device driver module for Sensirion SCD4x Co2 and Temperature and Humidity Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Possible I2C slave address
#define SCD4X_I2C_ADDR_1 (0x62)

/**
 * @brief SCD4x device initialization parameters
 */
typedef struct {
  uint8_t bus;  /**< I2C bus at which sensor is connected */
  uint8_t addr; /**< I2C slave address of the sensor */
  int sda_pin;  /**< I2C SDA Pin number */
  int scl_pin;  /**< I2C SCL Pin number */
} scd4x_params_t;

/**
 * @brief SCD4x sensor device data structure type
 */
typedef struct {
  uint8_t bus;        /**< I2C bus at which sensor is connected */
  uint8_t addr;       /**< I2C slave address of the sensor */
  int sda_pin;        /**< I2C sda pin number */
  int scl_pin;        /**< I2C scl pin number */
  uint16_t serial[3]; /**< sensor serial Address */
} scd4x_dev_t;

/**
 * @brief Intialize the SCD4x sensor device
 * This function initializes the sensor device.
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
int scd4x_init(scd4x_dev_t* dev, const scd4x_params_t* params);

/**
 * @brief get_data_ready_status I2C sequence description
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return int 0 on success, -1 on error
 */
int scd4x_get_data_ready_flag(scd4x_dev_t* dev);

/**
 * @brief read sensor output
 *
 * @param[in]   dev                 Device context of SCD4x device to be initialized
 * @param[out]  co2                 Co2 Sensor Data
 * @param[out]  temperature_deg_c   temperature Sensor Data
 * @param[out]  humidity_percent_rh humidity Sensor Data
 *
 * @return int 0 on success, -1 on error
 */
int scd4x_read_measurement(scd4x_dev_t* dev, uint16_t* co2, float* temperature_deg_c, float* humidity_percent_rh);

#ifdef __cplusplus
extern "C" {
#endif

#endif /* _SCD4X_H_ */
