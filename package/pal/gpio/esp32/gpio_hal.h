/** @file gpio_hal.h
 *
 * @brief GPIO HAL api to support esp32 gpio driver apis
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _GPIO_HAL_H_
#define _GPIO_HAL_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  HAL_GPIO_NO_ERR = 0,
  HAL_GPIO_INIT_ERR = -1,
  HAL_GPIO_WRITE_ERR = -2
} gpio_hal_err_t;

typedef enum {
  INPUT = 0,
  INPUT_PULLUP = 1,
  OUTPUT = 2
} gpio_hal_mode_t;

/**
 *
 */
int gpio_hal_init(uint8_t pin, uint8_t mode);

/**
 *
 */
int gpio_hal_read(uint8_t pin);

/**
 *
 */
int gpio_hal_write(uint8_t pin, uint8_t val);
#endif /* _GPIO_HAL_H_ */
