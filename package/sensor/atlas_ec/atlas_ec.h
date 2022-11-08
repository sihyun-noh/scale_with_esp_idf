/** @file atlas_ec.h
 *
 * @brief Device driver module for Atlas EC Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _ATLAS_EC_H_
#define _ATLAS_EC_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Possible I2C slave address
#define ATLAS_EC_I2C_ADDR_1 (0x64)

/**
 * @brief ATLAS_EC device initialization parameters
 */
typedef struct {
  uint8_t bus;  /**< I2C bus at which sensor is connected */
  uint8_t addr; /**< I2C slave address of the sensor */
  int sda_pin;  /**< I2C SDA Pin number */
  int scl_pin;  /**< I2C SCL Pin number */
} atlas_ec_params_t;

/**
 * @brief ATLAS_EC sensor device data structure type
 */
typedef struct {
  uint8_t bus;  /**< I2C bus at which sensor is connected */
  uint8_t addr; /**< I2C slave address of the sensor */
  int sda_pin;  /**< I2C sda pin number */
  int scl_pin;  /**< I2C scl pin number */
} atlas_ec_dev_t;

typedef enum {
  ATLAS_EC_CAL_DRY = 0,
  ATLAS_EC_CAL_SINGLE,
  ATLAS_EC_CAL_LOW,
  ATLAS_EC_CAL_HIGH,
  ATLAS_EC_CAL_CHECK,
  ATLAS_EC_CAL_CLEAR
} atlas_ec_cal_mode_t;

typedef enum { ATLAS_EC_PROBE_CHECK = 0, ATLAS_EC_PROBE_0_1, ATLAS_EC_PROBE_1_0, ATLAS_EC_PROBE_10 } atlas_ec_probe_t;

int atlas_ec_init(atlas_ec_dev_t* dev, const atlas_ec_params_t* params);
int atlas_ec_read(atlas_ec_dev_t* dev, char* ec);
int atlas_ec_cal(atlas_ec_dev_t* dev, atlas_ec_cal_mode_t mode);
int atlas_ec_probe(atlas_ec_dev_t* dev, atlas_ec_probe_t probe);

#ifdef __cplusplus
}
#endif

#endif /* _ATLAS_EC_H_ */
