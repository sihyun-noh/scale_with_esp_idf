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

#include "config.h"
#include "log.h"
#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
#include "sht3x_params.h"
#endif
#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
#include "scd4x_params.h"
#endif
#include "utils.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "monitoring.h"
#include "filelog.h"

#include <string.h>

#define MAX_BUFFER_CNT 10

static const char* TAG = "sensor_task";

#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
sht3x_dev_t dev;
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
scd4x_dev_t dev;
#endif

int sensor_init(void) {
  int res;

#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor = %d", res);
    return res;
  }
#elif defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
  // Please implement the SCD4x initialize code.
  if ((res = scd4x_init(&dev, &scd4x_params[0])) != 0) {
    LOGI(TAG, "Could not initialize SCD4X sensor = %d", res);
    return res;
  }
#endif

  return 0;
}

#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SHT3X)
int read_temperature_humidity(char* temperature, char* humidity) {
  int res = 0;
  int err_cnt = 0;

  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  float sht3x_temperature[MAX_BUFFER_CNT] = { 0 };
  float sht3x_humidity[MAX_BUFFER_CNT] = { 0 };

  float sht3x_temperature_i2c = 0;
  float sht3x_humidity_i2c = 0;

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

  memcpy(temperature, s_temperature, sizeof(s_temperature));
  memcpy(humidity, s_humidity, sizeof(s_humidity));

  return 0;
}
#endif

#if defined(SENSOR_TYPE) && (SENSOR_TYPE == SCD4X)
int read_co2_temperature_humidity(char* co2, char* temperature, char* humidity) {
  int res = 0;

  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  uint16_t scd41_co2 = 0;
  float scd41_temperature = 0;
  float scd41_humidity = 0;

  res = scd4x_get_data_ready_flag(&dev);
  if (res == -1) {
    LOGI(TAG, "scd4x is not data ready !!!");
    return res;
  }

  res = scd4x_read_measurement(&dev, &scd41_co2, &scd41_temperature, &scd41_humidity);

  if (res) {
    LOGI(TAG, "Error executing scd4x_read_measurement(): %i", res);
  } else if (scd41_co2 == 0) {
    LOGI(TAG, "Invalid sample detected, skipping.");
    res = -1;
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

    memcpy(co2, s_co2, sizeof(s_co2));
    memcpy(temperature, s_temperature, sizeof(s_temperature));
    memcpy(humidity, s_humidity, sizeof(s_humidity));
  }

  return res;
}
#endif

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
