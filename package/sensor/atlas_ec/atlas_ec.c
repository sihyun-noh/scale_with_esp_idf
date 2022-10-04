/** @file atlas_ec.c
 *
 * @brief Device driver module for Atlas EC Sensors
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <string.h>

#include "i2c_api.h"
#include "atlas_ec.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "log.h"

static const char* TAG = "atlas_ec";

int atlas_ec_init(atlas_ec_dev_t* dev, const atlas_ec_params_t* params) {
  int ret = 0;

  dev->bus = params->bus;
  dev->addr = params->addr;
  dev->sda_pin = params->sda_pin;
  dev->scl_pin = params->scl_pin;

  if ((ret = i2c_init(dev->bus, dev->sda_pin, dev->scl_pin)) != 0)
    LOGI(TAG, "Could not initialize, error = %d\n", ret);

  return ret;
}

int atlas_ec_read(atlas_ec_dev_t* dev, char* ec) {
  int ret = 0;
  uint8_t data[1] = "R";
  uint8_t read[10] = { 0 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  memcpy(ec, read+1, sizeof(read)-1);

  return ret;
}
