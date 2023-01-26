#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "log.h"
#include "led_strip.h"

#include "sys_status.h"

#define RMT_TX_GPIO 48
#define STRIP_LED_NUMBER 1
#define RMT_TX_CHANNEL RMT_CHANNEL_0

static const char *TAG = "led_task";
static TaskHandle_t led_handle = NULL;

led_strip_config_t strip_config;
led_strip_t *strip;

typedef enum {
  RED,
  GREEN,
  BLUE,
  WHITE,
  CLEAR
} stat_led_t;

static void init_led_strip(void) {
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_TX_GPIO, RMT_TX_CHANNEL);
  config.clk_div = 2;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

  strip_config.max_leds = STRIP_LED_NUMBER;
  strip_config.dev = (led_strip_dev_t)config.channel;
  strip = led_strip_new_rmt_sk68xx(&strip_config);

  if (!strip) {
    LOGE(TAG, "install SK68xx driver failed");
  }
  ESP_ERROR_CHECK(strip->clear(strip, 100));
}

static void set_led_strip(int r, int g, int b) {
  for (int i = 0; i < STRIP_LED_NUMBER; i++) {
    ESP_ERROR_CHECK(strip->set_pixel(strip, i, (uint32_t)r, (uint32_t)g, (uint32_t)b));
  }
  ESP_ERROR_CHECK(strip->refresh(strip, 100));
}

static void blink_led(stat_led_t color) {
  switch (color) {
    case RED: {
      set_led_strip(20, 0, 0);
    } break;
    case GREEN: {
      set_led_strip(0, 20, 0);
    } break;
    case BLUE: {
      set_led_strip(0, 0, 20);
    } break;
    case WHITE: {
      set_led_strip(10, 10, 10);
    } break;
    case CLEAR: {
      set_led_strip(0, 0, 0);
    } break;
    default: break;
  }
}

void led_task(void) {
  init_led_strip();

  while (1) {
    if (is_usb_copying(USB_COPYING)) {
      blink_led(GREEN);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (is_usb_copying(USB_COPY_FAIL)) {
      blink_led(RED);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (is_usb_copying(USB_COPY_SUCCESS)) {
      blink_led(BLUE);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    if (is_usb_disconnect_notify()) {
      blink_led(WHITE);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    blink_led(CLEAR);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void create_led_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (led_handle) {
    LOGI(TAG, "LED task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)led_task, "LED_TASK", stack_size, NULL, task_priority, &led_handle);
}
