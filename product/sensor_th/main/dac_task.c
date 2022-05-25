/* dac - Simple example

   Simple DAC example that shows how to initialize DAC
   as well as reading and writing from and to registers for a N-way valve connected over DAC.

   The 2way_valve used in this example.
*/

#include "nway_valve.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "esp_log.h"

#define NWAY_VALVE_CHANNEL 0  // GPIO25

static const char* TAG = "dac_task";

void dac_task(void* pvParameters) {
  uint8_t val = 0;

  nway_valve_init(NWAY_VALVE_CHANNEL);

  // Create a user task that uses the controllers.
  // xTaskCreate(user_dac_task, "user_dac_task", 2048, &dev, 2, 0);
  while (1) {
    // Get the values and do something with them.
    write_nway_valve(NWAY_VALVE_CHANNEL, val);
    ESP_LOGI(TAG, "dac value(%d)", val);
    // Wait until 2 seconds (cycle time) are over.
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}
