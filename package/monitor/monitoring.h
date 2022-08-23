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

#define MONITOR_QUEUE_SIZE 32
#define TASK_MAX_COUNT 100

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

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
