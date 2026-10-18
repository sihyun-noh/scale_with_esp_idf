#include "dajilog_ui_bridge.h"

#include <stdbool.h>
#include <string.h>

#include "freertos/FreeRTOS.h"

typedef struct {
    bool active;
    dajilog_ui_bridge_subscription_t id;
    dajilog_ui_bridge_observer_t observer;
    void *context;
} subscriber_t;

static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static bool s_initialized;
static dajilog_ui_bridge_config_t s_config;
static subscriber_t s_subscribers[DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS];
static dajilog_ui_bridge_subscription_t s_next_subscription = 1;

esp_err_t dajilog_ui_bridge_init(const dajilog_ui_bridge_config_t *config)
{
    if (config == NULL || config->command_handler == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&s_lock);
    if (s_initialized) {
        portEXIT_CRITICAL(&s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    s_config = *config;
    memset(s_subscribers, 0, sizeof(s_subscribers));
    s_initialized = true;
    portEXIT_CRITICAL(&s_lock);
    return ESP_OK;
}

esp_err_t dajilog_ui_bridge_deinit(void)
{
    portENTER_CRITICAL(&s_lock);
    memset(&s_config, 0, sizeof(s_config));
    memset(s_subscribers, 0, sizeof(s_subscribers));
    s_initialized = false;
    portEXIT_CRITICAL(&s_lock);
    return ESP_OK;
}

esp_err_t dajilog_ui_bridge_send_command(const dajilog_ui_msg_t *command)
{
    if (command == NULL || command->kind != DAJILOG_UI_MSG_COMMAND) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_initialized || s_config.command_handler == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return s_config.command_handler(command, s_config.command_context);
}

esp_err_t dajilog_ui_bridge_publish(const dajilog_ui_msg_t *message)
{
    if (message == NULL || message->kind == DAJILOG_UI_MSG_COMMAND) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    subscriber_t snapshot[DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS];
    portENTER_CRITICAL(&s_lock);
    memcpy(snapshot, s_subscribers, sizeof(snapshot));
    portEXIT_CRITICAL(&s_lock);

    for (size_t i = 0; i < DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS; ++i) {
        if (snapshot[i].active && snapshot[i].observer != NULL) {
            snapshot[i].observer(message, snapshot[i].context);
        }
    }
    return ESP_OK;
}

esp_err_t dajilog_ui_bridge_subscribe(
    dajilog_ui_bridge_observer_t observer,
    void *user_context,
    dajilog_ui_bridge_subscription_t *out_subscription)
{
    if (observer == NULL || out_subscription == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&s_lock);
    if (!s_initialized) {
        portEXIT_CRITICAL(&s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    for (size_t i = 0; i < DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS; ++i) {
        if (!s_subscribers[i].active) {
            dajilog_ui_bridge_subscription_t id = s_next_subscription++;
            if (s_next_subscription == 0) {
                s_next_subscription = 1;
            }
            s_subscribers[i] = (subscriber_t) {
                .active = true,
                .id = id,
                .observer = observer,
                .context = user_context,
            };
            *out_subscription = id;
            portEXIT_CRITICAL(&s_lock);
            return ESP_OK;
        }
    }
    portEXIT_CRITICAL(&s_lock);
    return ESP_ERR_NO_MEM;
}

esp_err_t dajilog_ui_bridge_unsubscribe(
    dajilog_ui_bridge_subscription_t subscription)
{
    if (subscription == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    portENTER_CRITICAL(&s_lock);
    for (size_t i = 0; i < DAJILOG_UI_BRIDGE_MAX_SUBSCRIBERS; ++i) {
        if (s_subscribers[i].active && s_subscribers[i].id == subscription) {
            memset(&s_subscribers[i], 0, sizeof(s_subscribers[i]));
            portEXIT_CRITICAL(&s_lock);
            return ESP_OK;
        }
    }
    portEXIT_CRITICAL(&s_lock);
    return ESP_ERR_NOT_FOUND;
}
