/** @file i2c_hal.h
 *
 * @brief I2C HAL api to support esp32 i2c driver apis
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _I2C_HAL_H_
#define _I2C_HAL_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  HAL_I2C_NO_ERR = 0,
  HAL_I2C_INIT_ERR = -1,
  HAL_I2C_READ_ERR = -2,
  HAL_I2C_WRITE_ERR = -3,
  HAL_I2C_INVALID_ARGS = -4,
  HAL_I2C_INVALID_IFACE = -5
} i2c_hal_err_t;

/**
 * @brief Initialize I2C bus and configuration
 * This function Initializes the I2C bus and configuration of ESP32
 *
 * @param[in] dev I2C device number
 *
 * @return
 *    - HAL_I2C_NO_ERR : Success
 *    - HAL_I2C_INIT_ERR : Fail
 */
int i2c_hal_init(int dev, int sda, int scl);

/**
 * @brief Free the I2C bus and configuration
 *
 * @param[in] dev I2C device number
 *
 * @return Success : HAL_I2C_NO_ERR
 */
int i2c_hal_free(int dev);

/**
 *
 */
void i2c_hal_lock(int dev);

/**
 *
 */
void i2c_hal_unlock(int dev);

/**
 *
 */
int i2c_hal_read(int dev, uint16_t addr, uint8_t *data, size_t len);

/**
 *
 */
int i2c_hal_write(int dev, uint16_t addr, const uint8_t *data, size_t len);

/**
 *
 */
int i2c_hal_write_regs(int dev, uint16_t addr, const void *reg, const uint8_t *data, size_t len);

#endif /* _I2C_HAL_H_ */
