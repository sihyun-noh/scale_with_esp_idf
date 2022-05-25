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

typedef struct _timezone {
  int index;
  char name[128];
  char tz[128];
} timezone_t;

/**
 * @brief Initialize the sntp client.
 *
 */
void tm_init_sntp(void);

/**
 * @brief Check if the timezone is already set.
 *
 * @return true timezone is already set.
 * @return false timezone is not set.
 */
bool tm_is_timezone_set(void);

/**
 * @brief Sync time with the ntp server.
 *
 */
void tm_apply_timesync(void);

/**
 * @brief Set the timezone to the system.
 *
 * @param tz the timezone to be set.
 */
void tm_set_timezone(const char *tz);

#endif /* _TIME_API_H_ */
