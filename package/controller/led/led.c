/** @file led.c
 *
 * @brief Device driver module for LED
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
#include "led.h"
#include "led_params.h"

#include "log.h"

static const char* TAG = "led";

int led_init(uint8_t pin) {
  int res = LED_OK;

  if ((res = gpio_init(pin, OUTPUT)) != 0) {
    LOGI(TAG, "Could not initialize, error = %d\n", __func__, res);
    return LED_ERR_GPIO;
  }

  return res;
}

int led_on(uint8_t pin) {
  int res = LED_OK;

  res = gpio_write(pin, 0);

  return res;
}

int led_off(uint8_t pin) {
  int res = LED_OK;

  res = gpio_write(pin, 1);

  return res;
}
