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

#ifdef __cplusplus
extern "C" {
#endif

#define MONITOR_QUEUE_SIZE 32

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

TaskHandle_t get_monitoring_task_handle(void);

#ifdef __cplusplus
}
#endif

#endif /* _MONITORING_H_ */
