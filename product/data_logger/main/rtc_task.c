#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ds3231.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <sys/time.h>
#include "esp_system.h"
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
  struct tm time = { .tm_year = year - 1900,  // since 1900
                     .tm_mon = month - 1,     // 0-based
                     .tm_mday = day,
                     .tm_hour = hour,
                     .tm_min = min,
                     .tm_sec = sec };
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

int rtc_set_time_cmd(int argc, char** argv) {
  struct tm time = { 0 };

  if (argc != 2) {
    printf("Usage: yyyy-mm-dd-hh-mm-ss <ex:2022-11-11-17-30-59>\n");
    return -1;
  }
  sscanf(argv[1], "%d-%d-%d-%d-%d-%d", &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min,
         &time.tm_sec);

  if (time.tm_year < 2022)
    return -1;
  if (time.tm_mon == 0 || time.tm_mon > 12)
    return -1;
  if (time.tm_mday == 0 || time.tm_mday > 31)
    return -1;
  if (time.tm_hour > 23)
    return -1;
  if (time.tm_min > 59)
    return -1;
  if (time.tm_sec > 59)
    return -1;

  rtc_set_time(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);

  return 0;
}

int rtc_get_time_cmd(int argc, char** argv) {
  struct tm time = { 0 };

  rtc_get_time(&time);
  printf("TIME: %04d-%02d-%02d-%02d-%02d-%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour,
         time.tm_min, time.tm_sec);

  return 0;
}

void set_sys_time_with_ds3231() {
  struct tm time = { 0 };
  time_t m_time;
  rtc_get_time(&time);
  printf("TIME: %04d-%02d-%02d-%02d-%02d-%02d\n", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour,
         time.tm_min, time.tm_sec);
  m_time = mktime(&time);
  settimeofday(&m_time, NULL);
}

