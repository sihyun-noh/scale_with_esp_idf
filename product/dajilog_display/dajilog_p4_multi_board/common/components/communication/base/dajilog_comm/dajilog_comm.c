#include "dajilog_comm.h"

#include <stdbool.h>
#include <string.h>

typedef struct {
    bool registered;
    const dajilog_comm_backend_ops_t *ops;
    void *context;
} backend_entry_t;

static backend_entry_t s_backends[DAJILOG_COMM_BACKEND_COUNT];
static dajilog_comm_event_handler_t s_event_handler;
static void *s_event_context;

static bool valid_type(dajilog_comm_type_t type)
{
    return type >= DAJILOG_COMM_CAN && type < DAJILOG_COMM_BACKEND_COUNT;
}

esp_err_t dajilog_comm_register_backend(dajilog_comm_type_t type,
                                        const dajilog_comm_backend_ops_t *ops,
                                        void *backend_context)
{
    if (!valid_type(type) || ops == NULL || ops->send == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_backends[type].registered) {
        return ESP_ERR_INVALID_STATE;
    }

    s_backends[type].registered = true;
    s_backends[type].ops = ops;
    s_backends[type].context = backend_context;
    return ESP_OK;
}

esp_err_t dajilog_comm_start(dajilog_comm_type_t type)
{
    if (!valid_type(type)) {
        return ESP_ERR_INVALID_ARG;
    }
    backend_entry_t *entry = &s_backends[type];
    if (!entry->registered) {
        return ESP_ERR_NOT_FOUND;
    }
    if (entry->ops->init != NULL) {
        esp_err_t error = entry->ops->init(entry->context);
        if (error != ESP_OK) {
            return error;
        }
    }
    return entry->ops->start != NULL ? entry->ops->start(entry->context) : ESP_OK;
}

esp_err_t dajilog_comm_stop(dajilog_comm_type_t type)
{
    if (!valid_type(type)) {
        return ESP_ERR_INVALID_ARG;
    }
    backend_entry_t *entry = &s_backends[type];
    if (!entry->registered) {
        return ESP_ERR_NOT_FOUND;
    }
    return entry->ops->stop != NULL ? entry->ops->stop(entry->context) : ESP_OK;
}

esp_err_t dajilog_comm_send(const dajilog_comm_request_t *request)
{
    if (request == NULL || !valid_type(request->type) ||
        request->payload_size > DAJILOG_COMM_PAYLOAD_CAPACITY) {
        return ESP_ERR_INVALID_ARG;
    }
    backend_entry_t *entry = &s_backends[request->type];
    if (!entry->registered) {
        return ESP_ERR_NOT_FOUND;
    }
    return entry->ops->send(entry->context, request);
}

esp_err_t dajilog_comm_get_state(dajilog_comm_type_t type,
                                 dajilog_comm_state_t *out_state)
{
    if (!valid_type(type) || out_state == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    backend_entry_t *entry = &s_backends[type];
    if (!entry->registered) {
        return ESP_ERR_NOT_FOUND;
    }
    if (entry->ops->get_state == NULL) {
        *out_state = DAJILOG_COMM_STOPPED;
        return ESP_OK;
    }
    return entry->ops->get_state(entry->context, out_state);
}

void dajilog_comm_set_event_handler(dajilog_comm_event_handler_t handler,
                                    void *user_context)
{
    s_event_handler = handler;
    s_event_context = user_context;
}

void dajilog_comm_publish_event(const dajilog_comm_event_t *event)
{
    if (event != NULL && s_event_handler != NULL) {
        s_event_handler(event, s_event_context);
    }
}
