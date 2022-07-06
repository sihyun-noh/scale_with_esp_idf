/**
 * @file sysevent.c
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
#include "esp32/sysevent_impl.h"
#include "sysevent.h"

static sysevent_ctx_t* ctx = NULL;

int sysevent_create(void) {
  ctx = sysevent_create_impl();
  if (ctx == NULL) {
    return -1;
  }
  return 0;
}

void sysevent_destroy(void) {
  if (ctx != NULL) {
    sysevent_destroy_impl(ctx);
    ctx = NULL;
  }
}

int sysevent_set(int event_id, void* event_data) {
  return sysevent_set_impl(ctx, event_id, event_data);
}

int sysevent_get(const char* event_base, int event_id, void* event_data, size_t event_data_len) {
  return sysevent_get_impl(ctx, event_base, event_id, event_data, event_data_len);
}

int sysevent_get_with_handler(const char* event_base, int event_id, event_handler_t event_handler, void* handler_data) {
  return sysevent_get_with_handler_impl(ctx, event_base, event_id, event_handler, handler_data);
}

int sysevent_unregister_handler(const char* event_base, int event_id, event_handler_t event_handler) {
  return sysevent_unregister_handler_impl(ctx, event_base, event_id, event_handler);
}
