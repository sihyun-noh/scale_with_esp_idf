/** @file battery.c
 *
 * @brief Device driver module for BATTERY
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "adc_api.h"
#include "battery.h"

#include "esp_log.h"

static const char *TAG = "battery";

bool battery_init(void) {
  bool cali_enable;

  cali_enable = adc_calibration_init();

  ESP_LOGI(TAG, "BATTERY Initialize success. cal(%d)\n", cali_enable);

  return cali_enable;
}

int read_battery(uint8_t chan) {
  int val = 0;

  val = adc_read(chan);

  return val;
}
