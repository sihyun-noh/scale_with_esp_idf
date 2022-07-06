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

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

#include "esp_log.h"

#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#define DEBUG(...)            \
  do {                        \
    DEBUG_PRINT(__VA_ARGS__); \
  } while (0)

#define TASK_DELAY_MS(x) ((x) / portTICK_PERIOD_MS)
#define TASK_DELAY_SEC(x) ((x)*1000 / portTICK_PERIOD_MS)

int led_init(void) {
  int res = LED_OK;

  if ((res = gpio_init(LED_PARAM_RED1, OUTPUT)) != 0) {
    DEBUG("%s : Could not initialize, error = %d\n", __func__, res);
    return LED_ERR_GPIO;
  }

  if ((res = gpio_init(LED_PARAM_RED2, OUTPUT)) != 0) {
    DEBUG("%s : Could not initialize, error = %d\n", __func__, res);
    return LED_ERR_GPIO;
  }

  if ((res = gpio_init(LED_PARAM_WHITE, OUTPUT)) != 0) {
    DEBUG("%s : Could not initialize, error = %d\n", __func__, res);
    return LED_ERR_GPIO;
  }

  DEBUG("%s : LED Initialize success\n", __func__);

  led_on(LED_PARAM_RED1);
  led_on(LED_PARAM_RED2);
  led_on(LED_PARAM_WHITE);

  return res;
}

int led_on(uint8_t pin) {
  int res = LED_OK;

  res = gpio_write(pin, 1);

  return res;
}

int led_off(uint8_t pin) {
  int res = LED_OK;

  res = gpio_write(pin, 0);

  return res;
}
