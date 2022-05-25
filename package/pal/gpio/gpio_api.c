/** @file gpio_api.c
 *
 * @brief gpio peripheral abstraction api that will be used in application layer
 *
 * This gpio api is designed to support various gpio drivers
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "gpio_api.h"
#include "esp32/gpio_hal.h"

int gpio_init(uint8_t pin, uint8_t mode) {
  return gpio_hal_init(pin, mode);
}

int gpio_read(uint8_t pin) {
  return gpio_hal_read(pin);
}

int gpio_write(uint8_t pin, uint8_t val) {
  return gpio_hal_write(pin, val);
}
