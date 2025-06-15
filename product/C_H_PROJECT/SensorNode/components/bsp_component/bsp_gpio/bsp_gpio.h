#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
/**
 * @brief Enumeration for LED colors used in the system.
 */
typedef enum { 
  LED_BLUE = 0x01,   ///< Blue LED identifier
  LED_GREEN = 0x02,         ///< Green LED identifier
  LED_RED = 0x04            ///< Red LED identifier
} led_color_t;
/* clang-format on */

/**
 * @brief Initializes the GPIO pins used for LED control.
 *
 * This function configures the necessary GPIOs as outputs
 * for driving the system status LEDs.
 *
 * @return ESP_OK on success, or an appropriate error code.
 */
esp_err_t bsp_gpio_init(void);

/**
 * @brief Blinks the status LEDs in a predefined pattern.
 *
 * This function can be used for indicating system states such as boot-up,
 * errors, or activity by blinking available LEDs.
 */
void blink_status_leds(void);

/**
 * @brief Turns on the specified LED(s).
 *
 * @param color The color (or combination of colors) to turn on.
 *              Use bitwise OR to turn on multiple LEDs (e.g., `LED_BLUE | LED_RED`).
 */
void blink_status_set_leds(led_color_t color);

/**
 * @brief Turns off the specified LED(s).
 *
 * @param color The color (or combination of colors) to turn off.
 *              Use bitwise OR to turn off multiple LEDs.
 */
void blink_status_clear_leds(led_color_t color);

#ifdef __cplusplus
}
#endif

#endif
