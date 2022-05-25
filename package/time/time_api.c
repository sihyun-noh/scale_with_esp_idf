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
#include "esp_sntp.h"
#include "log.h"
#include "time_api.h"

static const char *TAG = "time";

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

static void time_sync_notification_cb(struct timeval *tv) {
  LOGI(TAG, "Notification of a time synchronization event");
}

bool tm_is_timezone_set(void) {
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  // Is timezone set? if not, tm_year will be (1970 - 1900)
  if (timeinfo.tm_year < (2016 - 1900)) {
    LOGI(TAG, "Timezone is not set");
    return false;
  }
  LOGI(TAG, "Timezone is set");
  return true;
}

void tm_init_sntp(void) {
  LOGI(TAG, "Initializing SNTP");
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0, "pool.ntp.org");
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  sntp_init();
}

void tm_apply_timesync(void) {
  time_t now = 0;
  struct tm timeinfo = { 0 };
  int retry = 0;
  const int retry_count = 20;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
    LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  time(&now);
  localtime_r(&now, &timeinfo);
}

void tm_set_timezone(const char *tz) {
  time_t now;
  struct tm timeinfo;
  timezone_t *tz_info = NULL;
  char time_buff[64] = { 0 };

  int arr_size = sizeof(timezone_list) / sizeof(timezone_list[0]);
  for (int i = 0; i < arr_size; i++) {
    if (strcmp(tz, timezone_list[i].tz) == 0) {
      LOGI(TAG, "Setting timezone to %s", timezone_list[i].name);
      tz_info = &timezone_list[i];
    }
  }

  if (tz_info == NULL) {
    LOGE(TAG, "Invalid timezone");
    return;
  }

  setenv("TZ", tz_info->name, 1);
  tzset();
  localtime_r(&now, &timeinfo);
  strftime(time_buff, sizeof(time_buff), "%c", &timeinfo);
  LOGI(TAG, "The current date/time is: %s", time_buff);
}
