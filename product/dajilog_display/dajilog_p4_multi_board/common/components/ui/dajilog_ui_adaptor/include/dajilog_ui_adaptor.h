#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "dajilog_ui_bridge.h"
#include "dajilog_ui_msg.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_UI_ADAPTOR_DEFAULT_QUEUE_LENGTH 8U

typedef void (*dajilog_ui_adaptor_render_t)(
    const dajilog_ui_msg_t *message,
    void *user_context);

/**
 * Per-screen/app adaptor instance. The application owns this structure and
 * keeps it alive from attach until detach completes.
 */
typedef struct {
    void *queue_handle;
    dajilog_ui_bridge_subscription_t subscription;
    dajilog_ui_adaptor_render_t render;
    void *render_context;
    bool attached;
} dajilog_ui_adaptor_t;

typedef struct {
    size_t queue_length;
    dajilog_ui_adaptor_render_t render;
    void *render_context;
} dajilog_ui_adaptor_config_t;

esp_err_t dajilog_ui_adaptor_attach(
    dajilog_ui_adaptor_t *adaptor,
    const dajilog_ui_adaptor_config_t *config);

esp_err_t dajilog_ui_adaptor_detach(dajilog_ui_adaptor_t *adaptor);

/**
 * Drain pending Bridge messages and invoke the project render callback.
 * Call only from the LVGL/EEZ UI context, normally from an LVGL timer.
 */
size_t dajilog_ui_adaptor_process(dajilog_ui_adaptor_t *adaptor,
                                  size_t max_messages);

/** Submit a fully typed, project-owned command message to the Bridge. */
esp_err_t dajilog_ui_adaptor_submit(const dajilog_ui_msg_t *command);

#ifdef __cplusplus
}
#endif
