/**
 * @file time_api.h
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
#ifndef _TIME_API_H_
#define _TIME_API_H_

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KR_GMT_OFFSET 9  // UTC-9
#define KR_DST_OFFSET 0  // No DST setting in Korea

typedef struct _timezone {
  int index;
  char name[128];
  char tz[128];
} timezone_t;

/**
 * @brief Set local time.
 *
 * @param year
 * @param month
 * @param day
 * @param hour
 * @param min
 * @param sec
 *
 */
void set_local_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);

/**
 * @brief Set NTP server with gmt and dst offset to get NTP time from the NTP servers.
 *
 * @param gmt_offset_sec
 * @param dst_offset_sec
 * @param server1 the NTP server #1 that will be used to get NTP time
 * @param server2 the NTP server #2 that will be used to get NTP time
 * @param server3 the NTP server #3 that will be used to get NTP time
 *
 */
void tm_set_time(long gmt_offset_sec, int dst_offset_sec, const char* server1, const char* server2,
                 const char* server3);

/**
 * @brief Set NTP server with timezone to get NTP time from the NTP servers.
 *
 * @param tz the timezone to be set.
 * @param server1 the NTP server #1 that will be used to get NTP time
 * @param server2 the NTP server #2 that will be used to get NTP time
 * @param server3 the NTP server #3 that will be used to get NTP time
 *
 */
void tm_set_tztime(const char* tz, const char* server1, const char* server2, const char* server3);

/**
 * @brief Get NTP time and check if the result is successful or not.
 *
 * @return true : update success, false : failure
 */
bool tm_get_local_time(struct tm* info, uint32_t ms);

/**
 * @brief Set the current time to the system time.
 *
 */
void set_ntp_time(void);

bool get_ntp_time(int tz_offset, int dst_offset);

bool is_elapsed_uptime(uint32_t start_ms, uint32_t uptime_ms);

unsigned long millis(void);

void delay_ms(uint32_t ms);

#if defined(CONFIG_RTC_DS3231_PACKAGE)
void rtc_time_init(void);
void rtc_set_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
void rtc_get_time(struct tm* time);
#endif

int rtc_set_time_cmd(int argc, char** argv);
int rtc_get_time_cmd(int argc, char** argv);
int set_interval_cmd(int argc, char** argv);
int get_interval_cmd(int argc, char** argv);
int set_op_time_cmd(int argc, char** argv);
int get_op_time_cmd(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif /* _TIME_API_H_ */
