#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds3231.h>
#include <string.h>
#include "config.h"
#include "ds3231_params.h"
#include "syslog.h"

static const char* TAG = "rtc_task";
ds3231_dev_t rtc_dev;

void rtc_time_init(void) {
  memset(&rtc_dev, 0, sizeof(ds3231_dev_t));
  ESP_ERROR_CHECK(ds3231_init_desc(&rtc_dev, &ds3231_params[0]));
}

void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
  struct tm time = {
      .tm_year = year - 1900,  // since 1900
      .tm_mon = month - 1,     // 0-based
      .tm_mday = day,
      .tm_hour = hour,
      .tm_min = min,
      .tm_sec = sec
  };
  ESP_ERROR_CHECK(ds3231_set_time(&rtc_dev, &time));
}

void rtc_get_time(struct tm* time) {
  while (1) {
    vTaskDelay(pdMS_TO_TICKS(250));

    if (ds3231_get_time(&rtc_dev, time) != ESP_OK) {
      LOGI(TAG, "Could not get time");
      continue;
    }

    break;
  }
}
