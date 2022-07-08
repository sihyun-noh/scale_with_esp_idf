/** @file scd4x_params.h
 *
 * @brief Default configuration for Sensirion SCD4x CO2 and Temperature and Humidity Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _SCD4X_PARAMS_H_
#define _SCD4X_PARAMS_H_

#include "scd4x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default I2C device access macro
 */
#ifndef I2C_BUS
#define I2C_BUS(x) (x)
#endif

/**
 * @name    SCD4x default configuration parameters
 * @{
 */
#ifndef SCD4X_PARAM_I2C_BUS
#define SCD4X_PARAM_I2C_BUS (I2C_BUS(0))
#endif
#ifndef SCD4X_PARAM_I2C_ADDR
#define SCD4X_PARAM_I2C_ADDR (SCD4X_I2C_ADDR_1)
#endif
#ifndef SCD4X_PARAM_I2C_SDA
#define SCD4X_PARAM_I2C_SDA (SCD4X_I2C_SDA_PIN)
#endif
#ifndef SCD4X_PARAM_I2C_SCL
#define SCD4X_PARAM_I2C_SCL (SCD4X_I2C_SCL_PIN)
#endif

#ifndef SCD4X_PARAMS
#define SCD4X_PARAMS                                                                          \
  {                                                                                           \
    .bus = SCD4X_PARAM_I2C_BUS, .addr = SCD4X_PARAM_I2C_ADDR, .sda_pin = SCD4X_PARAM_I2C_SDA, \
    .scl_pin = SCD4X_PARAM_I2C_SCL                                                            \
  }
#endif
/**@}*/

/**
 * @brief   SCD4x configuration structure
 */
static const scd4x_params_t scd4x_params[] = { SCD4X_PARAMS };

#ifdef __cplusplus
}
#endif

#endif /* _SCD4X_PARAMS_H_ */
