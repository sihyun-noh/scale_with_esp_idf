/** @file dac_api.c
 *
 * @brief dac peripheral abstraction api that will be used in application layer
 *
 * This dac api is designed to support various dac drivers
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "dac_api.h"
#include "esp32/dac_hal.h"

void dac_init(uint8_t chan) {
  dac_hal_init(chan);
}

void dac_write(uint8_t chan, uint8_t value) {
  dac_hal_write(chan, value);
}
