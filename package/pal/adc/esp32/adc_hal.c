/** @file adc_hal.c
 *
 * @brief ADC HAL (peripheral abstraction api) that will be used in application layer
 *
 * This ADC api is designed to support various adc drivers
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

#include "adc_hal.h"

#include "driver/adc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/portmacro.h"
#include "esp_adc_cal.h"

// ADC Attenuation
#define ADC_ATTEN ADC_ATTEN_DB_11

// ADC Calibration
#define ADC_CALI_SCHEME ESP_ADC_CAL_VAL_EFUSE_VREF

static esp_adc_cal_characteristics_t adc1_chars;

bool adc_hal_calibration_init(void) {
  esp_err_t ret;
  bool cali_enable = false;

  ret = esp_adc_cal_check_efuse(ADC_CALI_SCHEME);
  if (ret == ESP_ERR_NOT_SUPPORTED) {
    printf("%s: Calibration scheme not supported, skip software calibration", __func__);
  } else if (ret == ESP_ERR_INVALID_VERSION) {
    printf("%s: eFuse not burnt, skip software calibration", __func__);
  } else if (ret == ESP_OK) {
    cali_enable = true;
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
  } else {
    printf("%s: Invalid arg", __func__);
  }

  return cali_enable;
}

int adc_hal_read(uint8_t chan) {
  int val = 0;

  if (chan >= ADC1_CHANNEL_MAX) {
    printf("%s: Invalid ADC channel", __func__);
    return 0;
  }

  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
  adc1_config_channel_atten(chan, ADC_ATTEN);

  val = adc1_get_raw(chan);

  return val;
}

int adc_hal_read_to_voltage(uint8_t chan) {
  int val = 0;
  uint32_t voltage;

  if (chan >= ADC1_CHANNEL_MAX) {
    printf("%s: Invalid ADC channel", __func__);
    return 0;
  }

  adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
  adc1_config_channel_atten(chan, ADC_ATTEN);

  val = adc1_get_raw(chan);
  voltage = esp_adc_cal_raw_to_voltage(val, &adc1_chars);

  return (int)voltage;
}
