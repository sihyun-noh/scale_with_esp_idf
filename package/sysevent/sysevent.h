/**
 * @file sysevent.h
 *
 * @brief system event implementation for application layer
 * Sysevent is designed for the purpose of triggering and receiving events between task applications using event_id.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef _SYSEVENT_H_
#define _SYSEVENT_H_

#include <stddef.h>

#include "esp_event.h"

#ifdef __cplusplus
extern "C" {
#endif

// Declare an event base
ESP_EVENT_DECLARE_BASE(SYSEVENT_BASE);

/**
 * @brief User handler function which will be called when event is received
 */
typedef int (*event_handler_t)(void *handler_data);

/**
 * @brief Create sysevent that will be used between task application.
 *        This function should be called once in main task.
 *        If it is called several times, there is no problem. (It will return the same context, use Singleton pattern)
 *
 * @return int 0 on success, -1 on failure
 */
int sysevent_create(void);

/**
 * @brief Destroy sysevent context
 *
 */
void sysevent_destroy(void);

/**
 * @brief Trigger an event with event id and event data
 *
 * @param event_id the event id to trigger
 * @param event_data the event data to trigger
 * @return int 0 on success, -1 on failure
 */
int sysevent_set(int event_id, void *event_data);

/**
 * @brief Get an event with event id and event data
 *
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_data the event data what user want to get
 * @param event_data_size the event data length
 * @return int
 */
int sysevent_get(const char *event_base, int event_id, void *event_data, size_t event_data_size);

/**
 * @brief Receive an event with event id and then run a handler function to handle the event
 *
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_handler the event handler function what the user wants to run when receiving the event
 * @param handler_data the data what the user wants to pass to the event handler function
 * @return int 0 on success, -1 on failure
 */
int sysevent_get_with_handler(const char *event_base, int event_id, event_handler_t event_handler, void *handler_data);

/**
 * @brief Unregister the event handler used by sysevent_get_with_handler when it is no longer used.
 *
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_handler the event handler function what the user wants to unregister if it is no longer used.
 * @return int 0 on success, -1 on failure
 */
int sysevent_unregister_handler(const char *event_base, int event_id, event_handler_t event_handler);

#ifdef __cplusplus
}
#endif

#endif /* _SYSEVENT_H_ */
