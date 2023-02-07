/** @file sht3x_params.h
 *
 * @brief Default configuration for Sensirion SHT3x Temperature and Humidity Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _SHT3X_PARAMS_H_
#define _SHT3X_PARAMS_H_

#include "sht3x.h"

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
 * @name    SHT3x default configuration parameters
 * @{
 */
#ifndef SHT3X_PARAM_I2C_BUS
#define SHT3X_PARAM_I2C_BUS (I2C_BUS(0))
#endif
#ifndef SHT3X_PARAM_I2C_ADDR
#define SHT3X_PARAM_I2C_ADDR (SHT3X_I2C_ADDR_1)
#endif
#ifndef SHT3X_PARAM_I2C_SDA
#define SHT3X_PARAM_I2C_SDA (SHT3X_I2C_SDA_PIN)
#endif
#ifndef SHT3X_PARAM_I2C_SCL
#define SHT3X_PARAM_I2C_SCL (SHT3X_I2C_SCL_PIN)
#endif
#ifndef SHT3X_PARAM_MODE
#define SHT3X_PARAM_MODE (SHT3X_PERIODIC_1MPS)
#endif
#ifndef SHT3X_PARAM_REPEAT
#define SHT3X_PARAM_REPEAT (SHT3X_HIGH)
#endif

#ifndef SHT3X_PARAMS
#define SHT3X_PARAMS                                                                          \
  {                                                                                           \
    .bus = SHT3X_PARAM_I2C_BUS, .addr = SHT3X_PARAM_I2C_ADDR, .sda_pin = SHT3X_PARAM_I2C_SDA, \
    .scl_pin = SHT3X_PARAM_I2C_SCL, .mode = SHT3X_PARAM_MODE, .repeat = SHT3X_PARAM_REPEAT    \
  }
#endif
/**@}*/

/**
 * @brief   SHT3x configuration structure
 */
static const sht3x_params_t sht3x_params[] = { SHT3X_PARAMS };

#ifdef __cplusplus
}
#endif

#endif /* _SHT3X_PARAMS_H_ */
