/* i2c - Simple example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected
   over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "event_ids.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "log.h"
#include "sht3x.h"
#include "sht3x_params.h"
#include "sysevent.h"
#include "syslog.h"

static const char* TAG = "sensor_task";

static TaskHandle_t sensor_handle = NULL;

float temperature = 0;
float humidity = 0;

void sensor_task(void* pvParameters) {
  sht3x_dev_t dev;

  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  int res;

  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor");
  } else {
    LOGI(TAG, "sensor_task start");
    // Create a user task that uses the sensors.
    // xTaskCreate(user_i2c_task, "user_i2c_task", 2048, &dev, 2, 0);
    while (1) {
      // Get the values and do something with them.
      if ((res = sht3x_read(&dev, &temperature, &humidity)) == SHT3X_OK) {
        LOGI(TAG, "Temperature [°C]: %.2f", temperature);
        LOGI(TAG, "Relative Humidity [%%]: %.2f", humidity);
      }

      memset(s_temperature, 0, sizeof(s_temperature));
      snprintf(s_temperature, sizeof(s_temperature), "%.2f", temperature);
      memset(s_humidity, 0, sizeof(s_humidity));
      snprintf(s_humidity, sizeof(s_humidity), "%.2f", humidity);
      sysevent_set(I2C_TEMPERATURE_EVENT, (char*)s_temperature);
      sysevent_set(I2C_HUMIDITY_EVENT, (char*)s_humidity);

      // Wait until 2 seconds (cycle time) are over.
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }

  vTaskDelete(NULL);
}

void create_sensor_task(void) {
  uint16_t stack_size = 4096;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (sensor_handle) {
    LOGI(TAG, "Sensor task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)sensor_task, "sensor_task", stack_size, NULL, task_priority, &sensor_handle);
}
