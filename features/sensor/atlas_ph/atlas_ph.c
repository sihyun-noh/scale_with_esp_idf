/** @file atlas_ph.c
 *
 * @brief Device driver module for Atlas pH Sensors
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
#include "atlas_ph.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "log.h"

static const char* TAG = "atlas_ph";

int atlas_ph_init(atlas_ph_dev_t* dev, const atlas_ph_params_t* params) {
  int ret = 0;

  dev->bus = params->bus;
  dev->addr = params->addr;
  dev->sda_pin = params->sda_pin;
  dev->scl_pin = params->scl_pin;

  if ((ret = i2c_init(dev->bus, dev->sda_pin, dev->scl_pin)) != 0)
    LOGI(TAG, "Could not initialize, error = %d\n", ret);

  return ret;
}

int atlas_ph_read(atlas_ph_dev_t* dev, char* ph) {
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

  memcpy(ph, read+1, sizeof(read)-1);

  return ret;
}

int atlas_ph_cal(atlas_ph_dev_t* dev, atlas_ph_cal_mode_t mode) {
  int ret = 0;
  uint8_t read[10] = { 0 };
  uint8_t data[20] = { 0 };

  LOGI(TAG, "0: low[4.00], 1: mid[6.86], 2: high[9.18], 3: cal check, 4: cal clear");

  if (mode == ATLAS_PH_CAL_LOW)
    snprintf((char *)data, sizeof(data), "Cal,low,4.00");
  else if (mode == ATLAS_PH_CAL_MID)
    snprintf((char *)data, sizeof(data), "Cal,mid,6.86");
  else if (mode == ATLAS_PH_CAL_HIGH)
    snprintf((char *)data, sizeof(data), "Cal,high,9.18");
  else if (mode == ATLAS_PH_CAL_CHECK)
    snprintf((char *)data, sizeof(data), "Cal,?");
  else if (mode == ATLAS_PH_CAL_CLEAR)
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
    if (mode == ATLAS_PH_CAL_CHECK)
      LOGI(TAG, "Altlas pH Cal : %s ", read);
    else if (mode == ATLAS_PH_CAL_CLEAR)
      LOGI(TAG, "Altlas pH Cal clear success");
    else
      LOGI(TAG, "Altlas pH Cal[%d] success", mode);
  } else {
    LOGI(TAG, "Altlas pH Cal[%d] failed : %d", mode, read[0]);
  }

  return ret;
}
