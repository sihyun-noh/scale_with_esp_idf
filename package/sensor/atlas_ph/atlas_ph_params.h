/** @file atlas_ph_params.h
 *
 * @brief Default configuration for Atlas pH Sensors
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _ATLAS_PH_PARAMS_H_
#define _ATLAS_PH_PARAMS_H_

#include "atlas_ph.h"

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
 * @name    Atlas_ph default configuration parameters
 * @{
 */
#ifndef ATLAS_PH_PARAM_I2C_BUS
#define ATLAS_PH_PARAM_I2C_BUS (I2C_BUS(0))
#endif
#ifndef ATLAS_PH_PARAM_I2C_ADDR
#define ATLAS_PH_PARAM_I2C_ADDR (ATLAS_PH_I2C_ADDR_1)
#endif
#ifndef ATLAS_PH_PARAM_I2C_SDA
#define ATLAS_PH_PARAM_I2C_SDA (ATLAS_PH_I2C_SDA_PIN)
#endif
#ifndef ATLAS_PH_PARAM_I2C_SCL
#define ATLAS_PH_PARAM_I2C_SCL (ATLAS_PH_I2C_SCL_PIN)
#endif

#ifndef ATLAS_PH_PARAMS
#define ATLAS_PH_PARAMS                                                                          \
  {                                                                                           \
    .bus = ATLAS_PH_PARAM_I2C_BUS, .addr = ATLAS_PH_PARAM_I2C_ADDR, .sda_pin = ATLAS_PH_PARAM_I2C_SDA, \
    .scl_pin = ATLAS_PH_PARAM_I2C_SCL                                                            \
  }
#endif
/**@}*/

/**
 * @brief   Atlas_ph configuration structure
 */
static const atlas_ph_params_t atlas_ph_params[] = { ATLAS_PH_PARAMS };

#ifdef __cplusplus
}
#endif

#endif /* _ATLAS_PH_PARAMS_H_ */
