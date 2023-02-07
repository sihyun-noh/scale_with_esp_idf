/** @file ds3231_params.h
 *
 * @brief Default configuration for DS3231 RTC module
 *
 * Created by Greenlabs, Smartfarm IoT Eco Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _DS3231_PARAMS_H_
#define _DS3231_PARAMS_H_

#include "sdkconfig.h"
#include "ds3231.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DS3231_PARAM_I2C_BUS
#define DS3231_PARAM_I2C_BUS (CONFIG_DS3231_I2C_BUS)
#endif
#ifndef DS3231_PARAM_I2C_ADDR
#define DS3231_PARAM_I2C_ADDR (CONFIG_DS3231_I2C_ADDR)
#endif
#ifndef DS3231_PARAM_I2C_SDA
#define DS3231_PARAM_I2C_SDA (CONFIG_DS3231_I2C_SDA_PIN)
#endif
#ifndef DS3231_PARAM_I2C_SCL
#define DS3231_PARAM_I2C_SCL (CONFIG_DS3231_I2C_SCL_PIN)
#endif

#ifndef DS3231_PARAMS
#define DS3231_PARAMS                                                                          \
  {                                                                                           \
    .bus = DS3231_PARAM_I2C_BUS, .addr = DS3231_PARAM_I2C_ADDR, \
    .sda_pin = DS3231_PARAM_I2C_SDA, .scl_pin = DS3231_PARAM_I2C_SCL \
  }
#endif
/**@}*/

/**
 * @brief   DS3231 configuration structure
 */
static const ds3231_dev_t ds3231_params[] = { DS3231_PARAMS };

#ifdef __cplusplus
}
#endif

#endif /* _DS3231_PARAMS_H_ */
