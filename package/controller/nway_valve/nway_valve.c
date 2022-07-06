/** @file nway_valve.c
 *
 * @brief Device driver module for N-way valve
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
#include "nway_valve.h"

#include "esp_log.h"

void nway_valve_init(uint8_t chan) {
  dac_init(chan);
}

void write_nway_valve(uint8_t chan, uint8_t value) {
  dac_write(chan, value);
}
