/** @file led_strip.h
 *
 * @brief Device driver for LED STRIP
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _LED_STRIP_H_
#define _LED_STRIP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

typedef struct led_strip_s led_strip_t;
typedef void *led_strip_dev_t;

struct led_strip_s {
  esp_err_t (*set_pixel)(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue);
  esp_err_t (*refresh)(led_strip_t *strip, uint32_t timeout_ms);
  esp_err_t (*clear)(led_strip_t *strip, uint32_t timeout_ms);
  esp_err_t (*del)(led_strip_t *strip);
};

typedef struct {
  uint32_t max_leds;   /*!< Maximum LEDs in a single strip */
  led_strip_dev_t dev; /*!< LED strip device (e.g. RMT channel, PWM channel, etc) */
} led_strip_config_t;

#define LED_STRIP_DEFAULT_CONFIG(number, dev_hdl) \
  { .max_leds = number, .dev = dev_hdl, }

led_strip_t *led_strip_new_rmt_sk68xx(const led_strip_config_t *config);
led_strip_t *led_strip_init(uint8_t channel, uint8_t gpio, uint16_t led_num);
esp_err_t led_strip_denit(led_strip_t *strip);

#ifdef __cplusplus
}
#endif

#endif /* _LED_STRIP_H_ */
