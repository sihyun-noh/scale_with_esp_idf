/**
 * @file sysevent_impl.h
 *
 * @brief system event implementation for esp32 platform
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
#ifndef _SYSEVENT_IMPL_H_
#define _SYSEVENT_IMPL_H_

#include "esp_err.h"
#include "esp_event.h"

#include "sysevent.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Event APIs for esp32 platform
 *
 */

// Declare an event base
// ESP_EVENT_DECLARE_BASE(SYSEVENT_BASE);

/**
 * @brief Sysevent context structure
 */
typedef struct sysevent_ctx sysevent_ctx_t;

/**
 * @brief Create sysevent context and initialize it
 *
 * @return sysevent_ctx_t* the sysevent context on success, NULL on failure
 */
sysevent_ctx_t *sysevent_create_impl(void);

/**
 * @brief Destroy sysevent context
 *
 * @param ctx the context of sysevent
 */
void sysevent_destroy_impl(sysevent_ctx_t *ctx);

/**
 * @brief Trigger an event with event id and event data
 *
 * @param ctx the context of sysevent
 * @param event_id the event id to trigger
 * @param event_data the event data to trigger
 * @return int 0 on success, -1 on failure
 */
int sysevent_set_impl(sysevent_ctx_t *ctx, int event_id, void *event_data);

/**
 * @brief Get an event with event id and event data
 *
 * @param ctx the context of sysevent
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_data the event data what user want to get
 * @param event_data_len the event data length
 * @return int 0 on success, -1 on failure
 */
int sysevent_get_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id, void *event_data,
                      size_t event_data_len);

/**
 * @brief Receive an event with event id and then run a handler function to handle the event
 *
 * @param ctx the context of sysevent
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_handler the event handler what user want to execute
 * @param handler_data the event handler data to pass to the event handler
 * @return int 0 on success, -1 on failure
 */
int sysevent_get_with_handler_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id,
                                   event_handler_t event_handler, void *handler_data);

/**
 * @brief Unregister the event handler used by sysevent_get_with_handler when it is no longer used.
 *
 * @param ctx the context of sysevent
 * @param event_base the event base what user want to get
 * @param event_id the event id what user want to get
 * @param event_handler the event handler function what the user wants to unregister if it is no longer used.
 * @return int 0 on success, -1 on failure
 */
int sysevent_unregister_handler_impl(sysevent_ctx_t *ctx, const char *event_base, int event_id,
                                     event_handler_t event_handler);

#ifdef __cplusplus
}
#endif

#endif /* _SYSEVENT_IMPL_H_ */
