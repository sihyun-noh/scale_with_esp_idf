/* i2c - Simple example

   Simple I2C example that shows how to initialize I2C
   as well as reading and writing from and to registers for a sensor connected over I2C.

   The sensor used in this example is a MPU9250 inertial measurement unit.

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   See README.md file to get detailed usage of this example.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "log.h"
#include "sht3x.h"
#include "sht3x_params.h"
#include "scd4x.h"
#include "utils.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "monitoring.h"

#include <string.h>

static const char* TAG = "sensor_task";
static TaskHandle_t i2c_handle = NULL;

void sht3x_task(void* pvParameters) {
  int res;
  sht3x_dev_t dev;

  char s_temperature[20];
  char s_humidity[20];

  float sht3x_temperature;
  float sht3x_humidity;

  is_run_task_monitor_alarm(i2c_handle);

  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor");
    i2c_handle = NULL;
    vTaskDelete(NULL);
    return;
  } else {
    LOGI(TAG, "i2c_task start");
    while (1) {
      is_run_task_heart_bit(i2c_handle, true);
      // Get the values and do something with them.
      if ((res = sht3x_read(&dev, &sht3x_temperature, &sht3x_humidity)) == SHT3X_OK) {
        LOGI(TAG, "Temperature [°C]: %.2f", sht3x_temperature);
        LOGI(TAG, "Relative Humidity [%%]: %.2f", sht3x_humidity);

        memset(s_temperature, 0, sizeof(s_temperature));
        snprintf(s_temperature, sizeof(s_temperature), "%.2f", sht3x_temperature);
        memset(s_humidity, 0, sizeof(s_humidity));
        snprintf(s_humidity, sizeof(s_humidity), "%.2f", sht3x_humidity);
        sysevent_set(I2C_TEMPERATURE_EVENT, (char*)s_temperature);
        sysevent_set(I2C_HUMIDITY_EVENT, (char*)s_humidity);
      }
      // Wait until 2000 msec (cycle time) are over.
      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
  vTaskDelete(NULL);
}

void scd4x_task(void* pvParameters) {
  int res;
  scd4x_dev_t dev;
  char s_co2[20];
  char s_temperature[20];
  char s_humidity[20];

  uint16_t scd41_co2;
  float scd41_temperature;
  float scd41_humidity;

  if ((res = scd4x_init(&dev)) != 0) {
    LOGI(TAG, "Could not initialize, error = %d", res);
    i2c_handle = NULL;
    vTaskDelete(NULL);
    return;
  } else {
    LOGI(TAG, "Waiting for first measurement... (5 sec)");

    while (1) {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      res = scd4x_get_data_ready_flag(&dev);
      if (!res) {
        LOGI(TAG, "scd4x get data ready");
        continue;
      }

      res = scd4x_read_measurement(&dev, &scd41_co2, &scd41_temperature, &scd41_humidity);
      if (res) {
        LOGI(TAG, "Error executing scd4x_read_measurement(): %i", res);
      } else if (scd41_co2 == 0) {
        LOGI(TAG, "Invalid sample detected, skipping.");
      } else {
        LOGI(TAG, "CO2: %u ppm", scd41_co2);
        LOGI(TAG, "Temperature [°C]: %.2f", scd41_temperature);
        LOGI(TAG, "Relative Humidity [%%]: %.2f", scd41_humidity);

        memset(s_co2, 0, sizeof(s_co2));
        snprintf(s_co2, sizeof(s_co2), "%d", scd41_co2);
        memset(s_temperature, 0, sizeof(s_temperature));
        snprintf(s_temperature, sizeof(s_temperature), "%.2f", scd41_temperature);
        memset(s_humidity, 0, sizeof(s_humidity));
        snprintf(s_humidity, sizeof(s_humidity), "%.2f", scd41_humidity);

        sysevent_set(I2C_CO2_EVENT, (char*)s_co2);
        sysevent_set(I2C_TEMPERATURE_EVENT, (char*)s_temperature);
        sysevent_set(I2C_HUMIDITY_EVENT, (char*)s_humidity);
      }
    }
  }
  vTaskDelete(NULL);
}

void create_i2c_task(uint16_t stack_size) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (i2c_handle) {
    LOGI(TAG, "i2c task is alreay created");
    return;
  }

  if (!strncmp(FW_VERSION, "SENS_SHT3x", 10)) {
    xTaskCreate((TaskFunction_t)sht3x_task, "sht3x_task", stack_size, NULL, task_priority, &i2c_handle);
  } else if (!strncmp(FW_VERSION, "SENS_SCD4x", 10)) {
    xTaskCreate((TaskFunction_t)scd4x_task, "scd4x_task", stack_size, NULL, task_priority, &i2c_handle);
  }
}

#define MAX_BUFFER_CNT 10
sht3x_dev_t dev;

int sensor_init(void) {
  int res;

  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor");
    return res;
  } else {
    return 0;
  }
}

int read_temp_humi(void) {
  int res;
  int err_cnt = 0;

  char s_temperature[20];
  char s_humidity[20];

  float sht3x_temperature[MAX_BUFFER_CNT];
  float sht3x_humidity[MAX_BUFFER_CNT];

  float sht3x_temperature_i2c;
  float sht3x_humidity_i2c;

  for (int i = 0; i < MAX_BUFFER_CNT;) {
    // Get the values and do something with them.
    if ((res = sht3x_read(&dev, &sht3x_temperature[i], &sht3x_humidity[i])) == SHT3X_OK) {
      LOGI(TAG, "Temperature [°C]: %.2f", sht3x_temperature[i]);
      LOGI(TAG, "Relative Humidity [%%]: %.2f", sht3x_humidity[i]);
      i++;
    } else {
      err_cnt++;
      if (err_cnt >= 100)
        return res;
    }
    // Wait until 100 msec (cycle time) are over.
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  qsort(sht3x_temperature, MAX_BUFFER_CNT, sizeof(float), float_compare);
  qsort(sht3x_humidity, MAX_BUFFER_CNT, sizeof(float), float_compare);

  sht3x_temperature_i2c = float_average(&sht3x_temperature[1], MAX_BUFFER_CNT - 2);
  sht3x_humidity_i2c = float_average(&sht3x_humidity[1], MAX_BUFFER_CNT - 2);

  LOGI(TAG, "average Temperature [°C]: %.2f", sht3x_temperature_i2c);
  LOGI(TAG, "average Relative Humidity [%%]: %.2f", sht3x_humidity_i2c);

  memset(s_temperature, 0, sizeof(s_temperature));
  snprintf(s_temperature, sizeof(s_temperature), "%.2f", sht3x_temperature_i2c);
  memset(s_humidity, 0, sizeof(s_humidity));
  snprintf(s_humidity, sizeof(s_humidity), "%.2f", sht3x_humidity_i2c);

  sysevent_set(I2C_TEMPERATURE_EVENT, (char*)s_temperature);
  sysevent_set(I2C_HUMIDITY_EVENT, (char*)s_humidity);
  return 0;
}

int temperature_comparison(float m_temperature, float temperature) {
  float comparison;

  comparison = temperature - m_temperature;

  LOGI(TAG, "temperature : %.2f, m_temperature : %.2f, comparison: %.2f", temperature, m_temperature, comparison);

  if (comparison < 0) {
    comparison = comparison * -1;
  }

  if (comparison >= 0.5) {
    return 1;
  }
  return 0;
}
