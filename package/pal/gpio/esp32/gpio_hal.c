/** @file gpio_hal.c
 *
 * @brief GPIO HAL (peripheral abstraction api) that will be used in application layer
 *
 * This GPIO api is designed to support various gpio drivers
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "gpio_hal.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/portmacro.h"
#include "hal/gpio_types.h"

int gpio_hal_init(uint8_t pin, uint8_t mode) {
  gpio_config_t io_conf = {};

  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.pin_bit_mask = (1ULL << pin);
  io_conf.pull_down_en = 0;
  io_conf.pull_up_en = 0;

  switch (mode) {
    case INPUT:
      io_conf.mode = GPIO_MODE_INPUT;
      break;
    case INPUT_PULLUP:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = 1;
      break;
    case OUTPUT:
      io_conf.mode = GPIO_MODE_OUTPUT;
      break;
    default:
      printf("%s : Invalid gpio mode - %d\n", __func__, mode);
      return HAL_GPIO_INIT_ERR;
  }
  gpio_config(&io_conf);
  return HAL_GPIO_NO_ERR;
}

int gpio_hal_read(uint8_t pin) {
  int val = 0;

  val = gpio_get_level(pin);

  return val;
}

int gpio_hal_write(uint8_t pin, uint8_t val) {
  esp_err_t rc;

  rc = gpio_set_level(pin, val);

  if (ESP_OK != rc) {
    return HAL_GPIO_WRITE_ERR;
  }

  return HAL_GPIO_NO_ERR;
}
