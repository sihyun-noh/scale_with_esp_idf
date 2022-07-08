/** @file sht3x.h
 *
 * @brief Device driver for Sensirion SHT3x Humidity and Temperature Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _SHT3X_H_
#define _SHT3X_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Possible I2C slave addressed
#define SHT3X_I2C_ADDR_1 (0x44) /**< ADDR pin connected to GND/VSS (default) */
#define SHT3X_I2C_ADDR_2 (0x45) /**< ADDR pin connected to VDD */

/**
 * @brief Error codes
 */
typedef enum {
  SHT3X_OK = 0,
  SHT3X_ERR_I2C = -1,
  SHT3X_ERR_MEASURE_CMD = -2,
  SHT3X_ERR_CRC = -3,
  SHT3X_ERR_STATUS = -4,
  SHT3X_ERR_PARAMS = -5,
} sht3x_err_code;

/**
 * @brief STH3x measurement modes
 */
typedef enum {
  SHT3X_SINGLE_SHOT = 0, /**< one single measurement */
  SHT3X_PERIODIC_05MPS,  /**< periodic with 0.5 measurements per second (mps) */
  SHT3X_PERIODIC_1MPS,   /**< periodic with   1 measurements per second (mps) */
  SHT3X_PERIODIC_2MPS,   /**< periodic with   2 measurements per second (mps) */
  SHT3X_PERIODIC_4MPS,   /**< periodic with   4 measurements per second (mps) */
  SHT3X_PERIODIC_10MPS   /**< periodic with  10 measurements per second (mps) */
} sht3x_mode_t;

/**
 * @brief SHT3x repeatability levels
 */

typedef enum {
  SHT3X_HIGH = 0, /**< high repeatability */
  SHT3X_MEDIUM,   /**< medium repeatability */
  SHT3X_LOW       /**< low repeatability */
} sht3x_repeat_t;

/**
 * @brief SHT3x device initialization parameters
 */
typedef struct {
  uint8_t bus;           /**< I2C bus at which sensor is connected */
  uint8_t addr;          /**< I2C slave address of the sensor */
  int sda_pin;           /**< I2C SDA Pin number */
  int scl_pin;           /**< I2C SCL Pin number */
  sht3x_mode_t mode;     /**< used measurement mode */
  sht3x_repeat_t repeat; /**< used repeatability */
} sht3x_params_t;

/**
 * @brief SHT3x sensor device data structure type
 */
typedef struct {
  uint8_t bus;  /**< I2C bus at which sensor is connected */
  uint8_t addr; /**< I2C slave address of the sensor */
  int sda_pin;  /**< I2C SDA Pin number */
  int scl_pin;  /**< I2C SCL Pin number */

  sht3x_mode_t mode;     /**< used measurement mode */
  sht3x_repeat_t repeat; /** used repeatability */

  bool meas_started;        /**< indicates whether measurement started */
  uint32_t meas_start_time; /**< start time of current measurement in us */
  uint32_t meas_duration;   /**< time in us until the result of the current measurement become available */
} sht3x_dev_t;

/**
 * @brief Intialize the SHT3x sensor device
 * This function initializes the sensor device.
 *
 * @param[in]   dev     Device context of SHT3x device to be initialized
 * @param[in]   params  SHT3x initialization parameters
 *
 * @return 0 on success , otherwise negative error code
 */
int sht3x_init(sht3x_dev_t *dev, const sht3x_params_t *params);

/**
 * @brief Read SHT3x measurement results (temperature and humidity)
 * This function returns the results of one measurement once they are available.
 *
 * @param[in]   dev           Device context of SHT3x device to read from
 * @param[in]   temperature   Temperature in hundreadths of a degree Celsius
 * @param[in]   humidity      Relative humidity in hundreadths of a percent
 *
 * @return  0 on success, otherwise negative error code
 */
int sht3x_read(sht3x_dev_t *dev, float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

#endif /* _SHT3X_H_ */
