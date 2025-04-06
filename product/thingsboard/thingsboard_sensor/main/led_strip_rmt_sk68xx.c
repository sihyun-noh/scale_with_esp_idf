/** @file led_strip_rmt_sk68xx.c
 *
 * @brief Device driver for LED STRIP SK68xx
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
#include <string.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "esp_attr.h"
#include "led_strip.h"
#include "driver/rmt.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0

static const char *TAG = "sk68xx";
#define STRIP_CHECK(a, str, goto_tag, ret_value, ...)                       \
  do {                                                                      \
    if (!(a)) {                                                             \
      ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
      ret = ret_value;                                                      \
      goto goto_tag;                                                        \
    }                                                                       \
  } while (0)

#define SK68xx_T0H_NS (300)
#define SK68xx_T0L_NS (900)
#define SK68xx_T1H_NS (900)
#define SK68xx_T1L_NS (300)
#define SK68xx_RESET_US (100)

static uint32_t sk68xx_t0h_ticks = 0;
static uint32_t sk68xx_t1h_ticks = 0;
static uint32_t sk68xx_t0l_ticks = 0;
static uint32_t sk68xx_t1l_ticks = 0;

typedef struct {
  led_strip_t parent;
  rmt_channel_t rmt_channel;
  uint32_t strip_len;
  uint8_t buffer[0];
} sk68xx_t;

/**
 * @brief Conver RGB data to RMT format.
 *
 * @note For SK68xx, R,G,B each contains 256 different choices (i.e. uint8_t)
 *
 * @param[in] src: source data, to converted to RMT format
 * @param[in] dest: place where to store the convert result
 * @param[in] src_size: size of source data
 * @param[in] wanted_num: number of RMT items that want to get
 * @param[out] translated_size: number of source data that got converted
 * @param[out] item_num: number of RMT items which are converted from source data
 */
static void IRAM_ATTR sk68xx_rmt_adapter(const void *src, rmt_item32_t *dest, size_t src_size, size_t wanted_num,
                                         size_t *translated_size, size_t *item_num) {
  if (src == NULL || dest == NULL) {
    *translated_size = 0;
    *item_num = 0;
    return;
  }
  const rmt_item32_t bit0 = { { { sk68xx_t0h_ticks, 1, sk68xx_t0l_ticks, 0 } } };  // Logical 0
  const rmt_item32_t bit1 = { { { sk68xx_t1h_ticks, 1, sk68xx_t1l_ticks, 0 } } };  // Logical 1
  size_t size = 0;
  size_t num = 0;
  uint8_t *psrc = (uint8_t *)src;
  rmt_item32_t *pdest = dest;
  while (size < src_size && num < wanted_num) {
    for (int i = 0; i < 8; i++) {
      // MSB first
      if (*psrc & (1 << (7 - i))) {
        pdest->val = bit1.val;
      } else {
        pdest->val = bit0.val;
      }
      num++;
      pdest++;
    }
    size++;
    psrc++;
  }
  *translated_size = size;
  *item_num = num;
}

static esp_err_t sk68xx_set_pixel(led_strip_t *strip, uint32_t index, uint32_t red, uint32_t green, uint32_t blue) {
  esp_err_t ret = ESP_OK;
  sk68xx_t *sk68xx = __containerof(strip, sk68xx_t, parent);
  STRIP_CHECK(index < sk68xx->strip_len, "index out of the maximum number of leds", err, ESP_ERR_INVALID_ARG);
  uint32_t start = index * 4;
  // In thr order of GRB
  sk68xx->buffer[start + 0] = green & 0xFF;
  sk68xx->buffer[start + 1] = red & 0xFF;
  sk68xx->buffer[start + 2] = blue & 0xFF;
  return ESP_OK;
err:
  return ret;
}

static esp_err_t sk68xx_refresh(led_strip_t *strip, uint32_t timeout_ms) {
  esp_err_t ret = ESP_OK;
  sk68xx_t *sk68xx = __containerof(strip, sk68xx_t, parent);
  STRIP_CHECK(rmt_write_sample(sk68xx->rmt_channel, sk68xx->buffer, sk68xx->strip_len * 4, true) == ESP_OK,
              "transmit RMT samples failed", err, ESP_FAIL);
  return rmt_wait_tx_done(sk68xx->rmt_channel, pdMS_TO_TICKS(timeout_ms));
err:
  return ret;
}

static esp_err_t sk68xx_clear(led_strip_t *strip, uint32_t timeout_ms) {
  sk68xx_t *sk68xx = __containerof(strip, sk68xx_t, parent);
  // Write zero to turn off all leds
  memset(sk68xx->buffer, 0, sk68xx->strip_len * 4);
  return sk68xx_refresh(strip, timeout_ms);
}

static esp_err_t sk68xx_del(led_strip_t *strip) {
  sk68xx_t *sk68xx = __containerof(strip, sk68xx_t, parent);
  free(sk68xx);
  return ESP_OK;
}

led_strip_t *led_strip_new_rmt_sk68xx(const led_strip_config_t *config) {
  led_strip_t *ret = NULL;
  STRIP_CHECK(config, "configuration can't be null", err, NULL);

  // 24 bits per led
  uint32_t sk68xx_size = sizeof(sk68xx_t) + config->max_leds * 4;
  sk68xx_t *sk68xx = calloc(1, sk68xx_size);
  STRIP_CHECK(sk68xx, "request memory for sk68xx failed", err, NULL);

  uint32_t counter_clk_hz = 0;
  STRIP_CHECK(rmt_get_counter_clock((rmt_channel_t)config->dev, &counter_clk_hz) == ESP_OK,
              "get rmt counter clock failed", err, NULL);
  // ns -> ticks
  float ratio = (float)counter_clk_hz / 1e9;
  sk68xx_t0h_ticks = (uint32_t)(ratio * SK68xx_T0H_NS);
  sk68xx_t0l_ticks = (uint32_t)(ratio * SK68xx_T0L_NS);
  sk68xx_t1h_ticks = (uint32_t)(ratio * SK68xx_T1H_NS);
  sk68xx_t1l_ticks = (uint32_t)(ratio * SK68xx_T1L_NS);

  // set sk68xx to rmt adapter
  rmt_translator_init((rmt_channel_t)config->dev, sk68xx_rmt_adapter);

  sk68xx->rmt_channel = (rmt_channel_t)config->dev;
  sk68xx->strip_len = config->max_leds;

  sk68xx->parent.set_pixel = sk68xx_set_pixel;
  sk68xx->parent.refresh = sk68xx_refresh;
  sk68xx->parent.clear = sk68xx_clear;
  sk68xx->parent.del = sk68xx_del;

  return &sk68xx->parent;
err:
  return ret;
}

led_strip_t *led_strip_init(uint8_t channel, uint8_t gpio, uint16_t led_num) {
  static led_strip_t *pStrip;

  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(gpio, channel);
  // set counter clock to 40MHz
  config.clk_div = 2;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

  // install sk68xx driver
  led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(led_num, (led_strip_dev_t)config.channel);

  pStrip = led_strip_new_rmt_sk68xx(&strip_config);

  if (!pStrip) {
    ESP_LOGE(TAG, "install SK68xx driver failed");
    return NULL;
  }

  // Clear LED strip (turn off all LEDs)
  ESP_ERROR_CHECK(pStrip->clear(pStrip, 100));

  return pStrip;
}

esp_err_t led_strip_denit(led_strip_t *strip) {
  sk68xx_t *sk68xx = __containerof(strip, sk68xx_t, parent);
  ESP_ERROR_CHECK(rmt_driver_uninstall(sk68xx->rmt_channel));
  return strip->del(strip);
}
