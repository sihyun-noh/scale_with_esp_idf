/** @file dac_hal.c
 *
 * @brief DAC HAL (peripheral abstraction api) that will be used in application layer
 *
 * This DAC api is designed to support various dac drivers
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <stdlib.h>

#include "dac_hal.h"

#include "driver/gpio.h"
#include "driver/dac.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/portmacro.h"

void dac_hal_init(uint8_t chan) {
  esp_err_t r;
  gpio_num_t dac_gpio_num;

  r = dac_pad_get_io_num(chan, &dac_gpio_num);
  assert(r == ESP_OK);

  printf("%s: DAC channel %d @ GPIO %d.\n", __func__, chan, dac_gpio_num);

  dac_output_enable(chan);
}

void dac_hal_write(uint8_t chan, uint8_t value) {
  dac_output_voltage(chan, value);
}
