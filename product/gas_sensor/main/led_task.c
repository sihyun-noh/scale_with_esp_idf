#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"
#include "log.h"
#include "sys_status.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000
#define RMT_LED_STRIP_GPIO_NUM 48
#define DELAY_1SEC 1000

static const char *TAG = "led_task";

static TaskHandle_t led_handle;

rmt_channel_handle_t led_chan;
rmt_tx_channel_config_t tx_chan_config;
rmt_encoder_handle_t led_encoder;
led_strip_encoder_config_t encoder_config;
rmt_transmit_config_t tx_config;
static uint8_t led_strip_pixels[3];

typedef enum { RED, GREEN, BLUE, WHITE, CLEAR } stat_led_t;

static void delay(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

static void init_led_strip(void) {
  LOGI(TAG, "Create RMT TX channel");
  tx_chan_config.clk_src = RMT_CLK_SRC_DEFAULT;
  tx_chan_config.gpio_num = RMT_LED_STRIP_GPIO_NUM;
  tx_chan_config.mem_block_symbols = 64;
  tx_chan_config.resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ;
  tx_chan_config.trans_queue_depth = 4;

  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));

  LOGI(TAG, "Install led strip encoder");
  encoder_config.resolution = RMT_LED_STRIP_RESOLUTION_HZ;

  ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));

  LOGI(TAG, "Enable RMT TX channel");
  ESP_ERROR_CHECK(rmt_enable(led_chan));

  tx_config.loop_count = 0;
}

static void set_led_strip(int red, int green, int blue) {
  led_strip_pixels[0] = green;
  led_strip_pixels[1] = red;
  led_strip_pixels[2] = blue;
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
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
      delay(DELAY_1SEC);
    }
    if (is_usb_copying(USB_COPY_FAIL)) {
      blink_led(RED);
      delay(DELAY_1SEC);
    }
    if (is_usb_copying(USB_COPY_SUCCESS)) {
      blink_led(BLUE);
      delay(DELAY_1SEC);
    }
    if (is_usb_disconnect_notify()) {
      blink_led(WHITE);
      delay(DELAY_1SEC);
    }
    blink_led(CLEAR);
    delay(DELAY_1SEC);
  }
}

void create_led_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (led_handle) {
    LOGE(TAG, "LED task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)led_task, "LED_TASK", stack_size, NULL, task_priority, &led_handle);
}
