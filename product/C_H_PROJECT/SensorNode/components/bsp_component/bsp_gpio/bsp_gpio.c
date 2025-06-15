#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "bsp_gpio.h"

static const char *TAG = "gpio_init";

/// @brief Status LED configuration structure.

typedef struct {
  gpio_num_t pin;          ///< GPIO pin number
  const char *color_name;  ///< Human-readable color name
  led_color_t mask;        ///< Bitmask for this LED (e.g., LED_BLUE = 0x01)
} status_led_t;

/// @brief Initialize GPIOs for status LEDs and input mode selectors.
/// @return ESP_OK if successful.
esp_err_t bsp_gpio_init() {
  // Configure output GPIOs (LEDs)
  gpio_config_t o_conf = { .pin_bit_mask = (1ULL << CONFIG_STATUS_LED_BLUE_PIN) |
                                           (1ULL << CONFIG_STATUS_LED_GREEN_PIN) | (1ULL << CONFIG_STATUS_LED_RED_PIN),
                           .mode = GPIO_MODE_INPUT_OUTPUT,
                           .pull_up_en = GPIO_PULLUP_ENABLE,
                           .intr_type = GPIO_INTR_DISABLE };
  gpio_config(&o_conf);

  // Set initial LED states to OFF (active-low)
  gpio_set_level((gpio_num_t)CONFIG_STATUS_LED_BLUE_PIN, 1);
  gpio_set_level((gpio_num_t)CONFIG_STATUS_LED_GREEN_PIN, 1);
  gpio_set_level((gpio_num_t)CONFIG_STATUS_LED_RED_PIN, 1);

  // Configure input GPIOs (WiFi/LAN selector)
  gpio_config_t i_conf = { .pin_bit_mask = (1ULL << CONFIG_WIFI_SELECT_PIN) | (1ULL << CONFIG_LAN_SELECT_PIN),
                           .mode = GPIO_MODE_INPUT,
                           .pull_up_en = GPIO_PULLUP_ENABLE,
                           .intr_type = GPIO_INTR_ANYEDGE };
  gpio_config(&i_conf);

  return ESP_OK;
}

/// @brief Predefined list of status LEDs (Blue, Green, Red)
static const status_led_t status_leds[] = { { .pin = GPIO_NUM_2, .color_name = "BLUE", .mask = LED_BLUE },
                                            { .pin = GPIO_NUM_4, .color_name = "GREEN", .mask = LED_GREEN },
                                            { .pin = GPIO_NUM_5, .color_name = "RED", .mask = LED_RED } };

/// @brief Blink each status LED in sequence with 1s delay.
void blink_status_leds(void) {
  const int delay_ms = 1000;

  for (size_t i = 0; i < sizeof(status_leds) / sizeof(status_leds[0]); ++i) {
    gpio_set_level(status_leds[i].pin, 0);  // Turn ON (active-low)
    ESP_LOGI(TAG, "Blinking LED: %s", status_leds[i].color_name);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
    gpio_set_level(status_leds[i].pin, 1);  // Turn OFF
  }
}

// e.g.
/* blink_status_set_leds(LED_BLUE | LED_RED);    // BLUE, RED ON */
/* blink_status_clear_leds(LED_GREEN | LED_RED); // GREEN, RED OFF */

/// @brief Turn ON the specified LED(s)
void blink_status_set_leds(led_color_t color_mask) {
  for (int i = 0; i < sizeof(status_leds) / sizeof(status_leds[0]); ++i) {
    if (color_mask & status_leds[i].mask) {
      gpio_set_level(status_leds[i].pin, 0);  // Turn ON (active-low)
      ESP_LOGI(TAG, "Set LED: %s", status_leds[i].color_name);
    }
  }
}

/// @brief Turn OFF the specified LED(s)
void blink_status_clear_leds(led_color_t color_mask) {
  for (int i = 0; i < sizeof(status_leds) / sizeof(status_leds[0]); ++i) {
    if (color_mask & status_leds[i].mask) {
      gpio_set_level(status_leds[i].pin, 1);  // Turn OFF (active-low)
      ESP_LOGI(TAG, "Clear LED: %s", status_leds[i].color_name);
    }
  }
}
