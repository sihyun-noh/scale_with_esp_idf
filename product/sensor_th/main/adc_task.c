/* adc - Simple example

   Simple ADC example that shows how to initialize ADC
   as well as reading and writing from and to registers for a BATTERY connected over ADC.

   The BATTERY used in this example is a simple BATTERY unit.
*/

#include "battery.h"

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "esp_log.h"

#define BATTERY_PORT 6  // GPIO34

static const char* TAG = "adc_task";

void adc_task(void* pvParameters) {
  int cal, val;

  cal = battery_init();
  if (cal) {
    ESP_LOGI(TAG, "calibration completed");
  } else {
    ESP_LOGI(TAG, "calibration not support");
  }
  // Create a user task that uses the sensors.
  // xTaskCreate(user_adc_task, "user_adc_task", 2048, &dev, 2, 0);
  while (1) {
    // Get the values and do something with them.
    val = read_battery(BATTERY_PORT);
    ESP_LOGI(TAG, "adc value(%d)", val);
    // Wait until 2 seconds (cycle time) are over.
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}
