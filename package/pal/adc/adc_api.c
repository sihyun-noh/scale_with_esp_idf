/** @file adc_api.c
 *
 * @brief adc peripheral abstraction api that will be used in application layer
 *
 * This adc api is designed to support various adc drivers
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
#include "esp32/adc_hal.h"

bool adc_calibration_init(void) {
  return adc_hal_calibration_init();
}

int adc_read(uint8_t chan) {
  return adc_hal_read(chan);
}
