/** @file atlas_ph.h
 *
 * @brief Device driver module for Atlas pH Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _ATLAS_PH_H_
#define _ATLAS_PH_H_

#include <stdio.h>

// Possible I2C slave address
#define ATLAS_PH_I2C_ADDR_1 (0x63)

/**
 * @brief ATLAS_PH device initialization parameters
 */
typedef struct {
  uint8_t bus;  /**< I2C bus at which sensor is connected */
  uint8_t addr; /**< I2C slave address of the sensor */
  int sda_pin;  /**< I2C SDA Pin number */
  int scl_pin;  /**< I2C SCL Pin number */
} atlas_ph_params_t;

/**
 * @brief ATLAS_PH sensor device data structure type
 */
typedef struct {
  uint8_t bus;        /**< I2C bus at which sensor is connected */
  uint8_t addr;       /**< I2C slave address of the sensor */
  int sda_pin;        /**< I2C sda pin number */
  int scl_pin;        /**< I2C scl pin number */
  uint16_t serial[3]; /**< sensor serial Address */
} atlas_ph_dev_t;

int atlas_ph_init(atlas_ph_dev_t* dev, const atlas_ph_params_t* params);
int atlas_ph_read(atlas_ph_dev_t* dev, char* ph);
#endif /* _ATLAS_PH_H_ */