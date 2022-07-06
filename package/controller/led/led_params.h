/** @file led_params.h
 *
 * @brief Default configuration for LED
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _LED_PARAMS_H_
#define _LED_PARAMS_H_

#include "led.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default GPIO device access macro
 */
#ifndef GPIO_NUM
#define GPIO_NUM(x) (x)
#endif

/**
 * @name    LED default configuration parameters
 * @{
 */
#ifndef LED_PARAM_RED1
#define LED_PARAM_RED1 (GPIO_NUM(18))
#endif
#ifndef LED_PARAM_RED2
#define LED_PARAM_RED2 (GPIO_NUM(5))
#endif
#ifndef LED_PARAM_WHITE
#define LED_PARAM_WHITE (GPIO_NUM(19))
#endif

typedef enum {
  INPUT = 0,
  INPUT_PULLUP = 1,
  OUTPUT = 2
} gpio_hal_mode_t;

#ifdef __cplusplus
}
#endif

#endif /* _LED_PARAMS_H_ */
