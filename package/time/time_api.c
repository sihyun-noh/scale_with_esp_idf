/**
 * @file time_api.c
 *
 * @brief The time api is designed to get the utc time from the ntp server.
 *        This api is used by the log message system to display the timestamp when the log occurred.
 *        And also used by other system components to get the current time.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_timer.h"
#include "esp_sntp.h"
#include "log.h"
#include "syscfg.h"
#include "sys_config.h"
#include "time_api.h"

#if defined(CONFIG_RTC_DS3231_PACKAGE)
#include "ds3231.h"
#include "ds3231_params.h"
ds3231_dev_t rtc_dev;
#endif

static bool s_ntp_failed = false;

static const char* TAG = "time";

timezone_t timezone_list[] = { { 1, "EST5EDT,M3.2.0,M11.1.0", "America/New_York" },
                               { 2, "PST8PDT,M3.2.0,M11.1.0", "America/Los_Angeles" },
                               { 3, "KST-9", "Asia/Seoul" },
                               { 4, "CST-8", "Asia/Shangai" },
                               { 5, "CST-8", "Asia/Taipei" },
                               { 6, "JST-9", "Asia/Tokyo" },
                               { 7, "<+04>-4", "Asia/Dubai" },
                               { 8, "<+07>-7", "Asia/Bangkok" },
                               { 9, "WIB-7", "Asia/Jakarta" },
                               { 10, "<+07>-7", "Asia/Ho_Chi_Minh" },
                               { 11, "IST-2IDT,M3.4.4/26,M10.5.0", "Asia/Jerusalem" },
                               { 12, "GMT0BST,M3.5.0/1,M10.5.0", "Europe/London" },
                               { 13, "CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Paris" },
                               { 14, "CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Berlin" },
                               { 15, "MSK-3", "Europe/Moscow" },
                               { 16, "IST-1GMT0,M10.5.0,M3.5.0/1", "Europe/Dublin" },
                               { 17, "WET0WEST,M3.5.0/1,M10.5.0", "Europe/Lisbon" },
                               { 18, "CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Madrid" },
                               { 19, "CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Rome" },
                               { 20, "CET-1CEST,M3.5.0,M10.5.0/3", "Europe/Amsterdam" } };

unsigned long millis(void) {
  return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

void delay_ms(uint32_t ms) {
  vTaskDelay(ms / portTICK_PERIOD_MS);
}

bool is_elapsed_uptime(uint32_t start_ms, uint32_t uptime_ms) {
  uint32_t now = millis();

  if (now - start_ms <= uptime_ms) {
    return false;
  }
  return true;
}

#if 0
static timezone_t* find_tzinfo(const char* tz) {
  timezone_t* tz_info = NULL;

  int arr_size = sizeof(timezone_list) / sizeof(timezone_list[0]);
  for (int i = 0; i < arr_size; i++) {
    if (strcmp(tz, timezone_list[i].tz) == 0) {
      LOGI(TAG, "Setting timezone to %s", timezone_list[i].name);
      tz_info = &timezone_list[i];
    }
  }
  return tz_info;
}
#endif

static void set_time_zone(long offset, int daylight) {
  char cst[17] = { 0 };
  char cdt[17] = "DST";
  char tz[33] = { 0 };

  if (offset % 3600) {
    sprintf(cst, "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
  } else {
    sprintf(cst, "UTC%ld", offset / 3600);
  }
  if (daylight != 3600) {
    long tz_dst = offset - daylight;
    if (tz_dst % 3600) {
      sprintf(cdt, "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
    } else {
      sprintf(cdt, "DST%ld", tz_dst / 3600);
    }
  }
  sprintf(tz, "%s%s", cst, cdt);
  LOGI(TAG, "tz = %s", tz);
  setenv("TZ", tz, 1);
  tzset();
}

void set_local_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
  struct tm time;

  time.tm_year = year - 1900;
  time.tm_mon = month - 1;
  time.tm_mday = day;
  time.tm_hour = hour;
  time.tm_min = min;
  time.tm_sec = sec;

  time_t t = mktime(&time);

  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);

  LOGI(TAG, "Setting time = %s", asctime(&time));
}

void tm_set_time(long gmt_offset_sec, int dst_offset_sec, const char* server1, const char* server2,
                 const char* server3) {
  if (sntp_enabled()) {
    sntp_stop();
  }
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char*)server1);
  sntp_setservername(1, (char*)server2);
  sntp_setservername(2, (char*)server3);
  // sntp_set_sync_interval(60 * 60 * 1000);  // Update time every hour
  sntp_init();
  set_time_zone(-gmt_offset_sec, dst_offset_sec);
}

void tm_set_tztime(const char* tz, const char* server1, const char* server2, const char* server3) {
  if (sntp_enabled()) {
    sntp_stop();
  }
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, (char*)server1);
  sntp_setservername(1, (char*)server2);
  sntp_setservername(2, (char*)server3);
  // sntp_set_sync_interval(60 * 60 * 1000);  // Update time every hour
  sntp_init();
  setenv("TZ", tz, 1);
  tzset();
}

bool tm_get_local_time(struct tm* info, uint32_t ms) {
  uint32_t start = millis();
  time_t now;

  while ((millis() - start) <= ms) {
    time(&now);
    localtime_r(&now, info);
    if (info->tm_year > (2016 - 1900)) {
      LOGI(TAG, "tm_get_local_time: Success to get NTP time !!!");
      return true;
    }
    delay_ms(10);
  }
  LOGI(TAG, "tm_get_local_time: Failed to get NTP time !!!");
  return false;
}

void set_ntp_time(void) {
  time_t now;

  uint16_t year = 0;
  uint8_t month = 0;
  uint8_t day = 0;
  uint8_t hour = 0;
  uint8_t min = 0;
  uint8_t sec = 0;

  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  // Get current time info and set them to variables.
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  min = timeinfo.tm_min;
  sec = timeinfo.tm_sec;

  set_local_time(year, month, day, hour, min, sec);
}

bool get_ntp_time(int tz_offset, int dst_offset) {
  uint16_t year = 0;
  uint8_t month = 0;
  uint8_t day = 0;
  uint8_t hour = 0;
  uint8_t min = 0;
  uint8_t sec = 0;

  struct tm timeinfo = { 0 };

  tm_get_local_time(&timeinfo, 5000);
  // Get current time info and set them to variables.
  year = timeinfo.tm_year + 1900;
  month = timeinfo.tm_mon + 1;
  day = timeinfo.tm_mday;
  hour = timeinfo.tm_hour;
  min = timeinfo.tm_min;
  sec = timeinfo.tm_sec;

  tm_set_time(3600 * tz_offset, 3600 * dst_offset, "pool.ntp.org", "time.google.com", "1.pool.ntp.org");

  // delay time 20 sec
  if (!tm_get_local_time(&timeinfo, 20000)) {
    s_ntp_failed = true;
    set_local_time(year, month, day, hour, min, sec);
  } else {
    s_ntp_failed = false;
  }
  return s_ntp_failed;
}

#if defined(CONFIG_RTC_DS3231_PACKAGE)
void rtc_time_init(void) {
  struct tm timeinfo;
  memset(&rtc_dev, 0, sizeof(ds3231_dev_t));
  ESP_ERROR_CHECK(ds3231_init_desc(&rtc_dev, &ds3231_params[0]));

  vTaskDelay(pdMS_TO_TICKS(250));
  rtc_get_time(&timeinfo);
  set_local_time(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                 timeinfo.tm_sec);
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
#endif

int rtc_set_time_cmd(int argc, char** argv) {
  struct tm time = { 0 };
  char timestamp[80] = { 0 };

  if (argc != 2) {
    printf("Usage: yyyy-mm-dd-hh-mm-ss <ex:2022-11-11-17-30-59>\n");
    return -1;
  }

  sscanf(argv[1], "%d-%d-%d-%d-%d-%d", &time.tm_year, &time.tm_mon, &time.tm_mday, &time.tm_hour, &time.tm_min,
         &time.tm_sec);
  snprintf(timestamp, sizeof(timestamp), "%d-%d-%d-%d-%d-%d", time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour,
           time.tm_min, time.tm_sec);

  syscfg_set(CFG_DATA, "rtc_set", timestamp);

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

#if defined(CONFIG_RTC_DS3231_PACKAGE)
  rtc_set_time(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
  set_local_time(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
#else
  set_local_time(time.tm_year, time.tm_mon, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec);
#endif

  return 0;
}

int rtc_get_time_cmd(int argc, char** argv) {
  struct tm timeinfo = { 0 };
  time_t now;

  time(&now);
  localtime_r(&now, &timeinfo);

  printf("TIME: %04d-%02d-%02d-%02d-%02d-%02d\n", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
         timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

  return 0;
}

int set_interval_cmd(int argc, char** argv) {
  int interval = 0;

  if (argc != 2) {
    printf("Usage: 1 ~ 3600 (sec)  <ex:set_interval 60>\n");
    return -1;
  }
  interval = atoi(argv[1]);
  if ((interval < 1) || (interval > 3600)) {
    printf("invalid argument!\n");
    return -1;
  }

  syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, argv[1]);

  return 0;
}

int get_interval_cmd(int argc, char** argv) {
  char s_send_interval[10] = { 0 };

  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  printf("INTERVAL: %d\n", atoi(s_send_interval));

  return 0;
}

int set_op_time_cmd(int argc, char** argv) {
  int s_op_time = 0;
  int s_op_start_time = 0;
  int s_op_end_time = 0;

  if (argc != 2) {
    printf("Usage: 00(start hour)00(end hour)(hour)  <ex:set_op_time 0618>\n");
    return -1;
  }
  s_op_time = atoi(argv[1]);
  s_op_start_time = s_op_time / 100;
  s_op_end_time = s_op_time % 100;

  if (s_op_time < 1) {
    printf("invalid argument!(time error)\n");
    return -1;
  }
  if (s_op_start_time < 0 || s_op_start_time > 24) {
    printf("invalid argument!(start set time error) \n");
    return -1;
  }
  if (s_op_end_time < 0 || s_op_end_time > 24 || s_op_start_time >= s_op_end_time) {
    printf("invalid argument!(end set time error) \n");
    return -1;
  }

  syscfg_set(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, argv[1]);

  return 0;
}

int get_op_time_cmd(int argc, char** argv) {
  char s_op_time[10] = { 0 };

  syscfg_get(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, s_op_time, sizeof(s_op_time));
  printf("OP_TIME: %d\n", atoi(s_op_time));
  return 0;
}
