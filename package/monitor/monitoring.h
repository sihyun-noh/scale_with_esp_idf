/** @file monitoring.h
 *
 * @brief This source code explains the implementation of the monitoring system and the concept of function.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _MONITORING_H_
#define _MONITORING_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "sys/queue.h"

#define reason2str(r) ((r > 176) ? system_event_reasons[r - 176] : system_event_reasons[r - 1])

#define MONITOR_QUEUE_SIZE 32
#define TASK_MAX_COUNT 100

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *system_event_reasons[] = { "UNSPECIFIED",
                                              "AUTH_EXPIRE",
                                              "AUTH_LEAVE",
                                              "ASSOC_EXPIRE",
                                              "ASSOC_TOOMANY",
                                              "NOT_AUTHED",
                                              "NOT_ASSOCED",
                                              "ASSOC_LEAVE",
                                              "ASSOC_NOT_AUTHED",
                                              "DISASSOC_PWRCAP_BAD",
                                              "DISASSOC_SUPCHAN_BAD",
                                              "UNSPECIFIED",
                                              "IE_INVALID",
                                              "MIC_FAILURE",
                                              "4WAY_HANDSHAKE_TIMEOUT",
                                              "GROUP_KEY_UPDATE_TIMEOUT",
                                              "IE_IN_4WAY_DIFFERS",
                                              "GROUP_CIPHER_INVALID",
                                              "PAIRWISE_CIPHER_INVALID",
                                              "AKMP_INVALID",
                                              "UNSUPP_RSN_IE_VERSION",
                                              "INVALID_RSN_IE_CAP",
                                              "802_1X_AUTH_FAILED",
                                              "CIPHER_SUITE_REJECTED",
                                              "BEACON_TIMEOUT",
                                              "NO_AP_FOUND",
                                              "AUTH_FAIL",
                                              "ASSOC_FAIL",
                                              "HANDSHAKE_TIMEOUT",
                                              "CONNECTION_FAIL" };

/**
 * @brief Create task that will be monitoring for wifi, heap, task.
 *
 * @return int 0 on success, -1 on failure.
 */

int create_monitoring_task(void);

/**
 * @brief Initialize the underlying monitoring
 *
 * @return int 0 on success, -1 on failure.
 */
int monitoring_init(void);

/**
 * @brief send to monitoring task the alarm signal of start task from current task.
 *
 * @param task_handle send to current tesk handle
 * @return int 0 on success, -1 on failure.
 */
int is_run_task_monitor_alarm(TaskHandle_t task_handle);

/**
 * @brief send to monitoring task the alarm signal of end task from current task.
 *
 * @param task_handle send to current tesk handle
 * @return int 0 on success, -1 on failure.
 */
int is_run_task_monitor_remove(TaskHandle_t task_handle);

/**
 * @brief send to monitoring task the run signal from current task.
 *        checking loop status.
 *
 * @param task_handle send to current tesk handle
 * @param status run signal is ture.
 */
void is_run_task_heart_bit(TaskHandle_t task_handle, uint8_t status);

#endif /* _MONITORING_H_ */
