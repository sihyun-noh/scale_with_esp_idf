#pragma once

#include <stddef.h>

#include "dajilog_ui_msg.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS 4U

typedef esp_err_t (*dajilog_ui_bridge_command_handler_t)(
    const dajilog_ui_msg_t *command,
    void *user_context);

typedef void (*dajilog_ui_bridge_observer_t)(
    const dajilog_ui_msg_t *message,
    void *user_context);

typedef struct {
    dajilog_ui_bridge_command_handler_t command_handler;
    void *command_context;
} dajilog_ui_bridge_config_t;

typedef uint32_t dajilog_ui_bridge_subscription_t;

/** @brief Initialize the message hub and install its command destination. */
esp_err_t dajilog_ui_bridge_init(const dajilog_ui_bridge_config_t *config);
/** @brief Deinitialize the bridge after all adaptors have unsubscribed. */
esp_err_t dajilog_ui_bridge_deinit(void);

/** @brief Route a UI command to the project-specific command mapper. */
esp_err_t dajilog_ui_bridge_send_command(const dajilog_ui_msg_t *command);

/** @brief Publish state, event, or result messages to all subscribers. */
esp_err_t dajilog_ui_bridge_publish(const dajilog_ui_msg_t *message);

/** @brief Register an observer and return its subscription handle. */
esp_err_t dajilog_ui_bridge_subscribe(
    dajilog_ui_bridge_observer_t observer,
    void *user_context,
    dajilog_ui_bridge_subscription_t *out_subscription);

/** @brief Remove a previously registered observer. */
esp_err_t dajilog_ui_bridge_unsubscribe(
    dajilog_ui_bridge_subscription_t subscription);

#ifdef __cplusplus
}
#endif
