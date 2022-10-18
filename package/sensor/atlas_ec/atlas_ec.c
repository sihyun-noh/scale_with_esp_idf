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

int atlas_ec_cal(atlas_ec_dev_t* dev, atlas_ec_cal_mode_t mode) {
  int ret = 0;
  uint8_t read[10] = { 0 };
  uint8_t data[20] = { 0 };

  LOGI(TAG, "0: dry, 1: single[1413], 2: low[84], 3: high[1413], 4: cal check, 5: cal clear");

  if (mode == ATLAS_EC_CAL_DRY)
    snprintf((char *)data, sizeof(data), "Cal,dry");
  else if (mode == ATLAS_EC_CAL_SINGLE)
    snprintf((char *)data, sizeof(data), "Cal,1413");
  else if (mode == ATLAS_EC_CAL_LOW)
    snprintf((char *)data, sizeof(data), "Cal,low,84");
  else if (mode == ATLAS_EC_CAL_HIGH)
    snprintf((char *)data, sizeof(data), "Cal,high,1413");
  else if (mode == ATLAS_EC_CAL_CHECK)
    snprintf((char *)data, sizeof(data), "Cal,?");
  else if (mode == ATLAS_EC_CAL_CLEAR)
    snprintf((char *)data, sizeof(data), "Cal,clear");
  else {
    return -1;
  }

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  if (read[0] == 1) {
    if (mode == ATLAS_EC_CAL_CHECK)
      LOGI(TAG, "Altlas EC Cal : %s ", read);
    else if (mode == ATLAS_EC_CAL_CLEAR)
      LOGI(TAG, "Altlas EC Cal clear success");
    else
      LOGI(TAG, "Altlas Ec Cal[%d] success", mode);
  } else {
    LOGI(TAG, "Altlas EC Cal[%d] failed : %d", mode, read[0]);
  }

  return ret;
}

int atlas_ec_probe(atlas_ec_dev_t* dev, atlas_ec_probe_t probe) {
  int ret = 0;
  uint8_t read[10] = { 0 };
  uint8_t data[20] = { 0 };

  LOGI(TAG, "0: check, 1: K[0.1], 2: K[1.0], 3: K[10]");

  if (probe == ATLAS_EC_PROBE_CHECK)
    snprintf((char *)data, sizeof(data), "K,?");
  else if (probe == ATLAS_EC_PROBE_0_1)
    snprintf((char *)data, sizeof(data), "K,0.1");
  else if (probe == ATLAS_EC_PROBE_1_0)
    snprintf((char *)data, sizeof(data), "K,1.0");
  else if (probe == ATLAS_EC_PROBE_10)
    snprintf((char *)data, sizeof(data), "K,10");
  else {
    return -1;
  }

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  if (read[0] == 1) {
    if (probe == ATLAS_EC_PROBE_CHECK)
      LOGI(TAG, "Altlas EC Probe : %s ", read);
    else
      LOGI(TAG, "Altlas EC Probe[%d] success", probe);
  } else {
    LOGI(TAG, "Altlas EC Probe[%d] failed : %d", probe, read[0]);
  }

  return ret;
}
