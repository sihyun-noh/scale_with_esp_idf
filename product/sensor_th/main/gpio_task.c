/* gpio - Simple example

   Simple GPIO example that shows how to initialize GPIO
   as well as reading and writing from and to registers for a LED connected over GPIO.

   The LED used in this example is a simple LED unit.

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "led.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "esp_log.h"

const char* TAG_GPIO = "gpio_task";

void gpio_task(void* pvParameters) {
  int res;

  if ((res = led_init()) != LED_OK) {
    ESP_LOGI(TAG_GPIO, "Could not initialize LED");
  } else {
    ESP_LOGI(TAG_GPIO, "gpio_task start");
    // Create a user task that uses the sensors.
    // xTaskCreate(user_gpio_task, "user_gpio_task", 2048, &dev, 2, 0);
    while (1) {
      // Get the values and do something with them.

      // Wait until 2 seconds (cycle time) are over.
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }

  vTaskDelete(NULL);
}
