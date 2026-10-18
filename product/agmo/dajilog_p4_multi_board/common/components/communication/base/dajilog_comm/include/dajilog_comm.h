#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_COMM_PAYLOAD_CAPACITY 64U

typedef enum {
    DAJILOG_COMM_CAN = 0,
    DAJILOG_COMM_RS485,
    DAJILOG_COMM_SDI12,
    DAJILOG_COMM_BACKEND_COUNT,
} dajilog_comm_type_t;

typedef enum {
    DAJILOG_COMM_STOPPED = 0,
    DAJILOG_COMM_STARTING,
    DAJILOG_COMM_ONLINE,
    DAJILOG_COMM_OFFLINE,
    DAJILOG_COMM_ERROR,
} dajilog_comm_state_t;

typedef struct {
    dajilog_comm_type_t type;
    uint32_t endpoint;
    uint32_t command;
    uint32_t sequence;
    uint8_t payload[DAJILOG_COMM_PAYLOAD_CAPACITY];
    size_t payload_size;
} dajilog_comm_request_t;

typedef struct {
    dajilog_comm_type_t type;
    uint32_t endpoint;
    uint32_t event;
    uint32_t sequence;
    esp_err_t error;
    uint8_t payload[DAJILOG_COMM_PAYLOAD_CAPACITY];
    size_t payload_size;
} dajilog_comm_event_t;

typedef void (*dajilog_comm_event_handler_t)(const dajilog_comm_event_t *event,
                                              void *user_context);

typedef struct {
    esp_err_t (*init)(void *backend_context);
    esp_err_t (*start)(void *backend_context);
    esp_err_t (*stop)(void *backend_context);
    esp_err_t (*send)(void *backend_context,
                      const dajilog_comm_request_t *request);
    esp_err_t (*get_state)(void *backend_context,
                           dajilog_comm_state_t *out_state);
} dajilog_comm_backend_ops_t;

/** Register a backend during system initialization, before concurrent use. */
esp_err_t dajilog_comm_register_backend(dajilog_comm_type_t type,
                                        const dajilog_comm_backend_ops_t *ops,
                                        void *backend_context);

esp_err_t dajilog_comm_start(dajilog_comm_type_t type);
esp_err_t dajilog_comm_stop(dajilog_comm_type_t type);
esp_err_t dajilog_comm_send(const dajilog_comm_request_t *request);
esp_err_t dajilog_comm_get_state(dajilog_comm_type_t type,
                                 dajilog_comm_state_t *out_state);

void dajilog_comm_set_event_handler(dajilog_comm_event_handler_t handler,
                                    void *user_context);

/** Called by a backend to publish a decoded receive/error event. */
void dajilog_comm_publish_event(const dajilog_comm_event_t *event);

#ifdef __cplusplus
}
#endif
