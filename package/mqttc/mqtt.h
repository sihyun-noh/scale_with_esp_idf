/**
 * @file mqtt.h
 *
 * @brief Implement generic MQTT API that will be used by the MQTT client
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef _MQTT_H_
#define _MQTT_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The context of the MQTT client
 */
typedef struct mqtt_ctx mqtt_ctx_t;

/**
 * @brief Event callback function for MQTT client
 */
typedef void (*mqtt_event_cb_fn_t)(void *handler_args, int32_t event_id, void *event_data);

/**
 * @brief Transport interface for MQTT client
 */
typedef enum {
  MQTT_UNKNOWN = 0,
  MQTT_TCP,
  MQTT_SSL,
  MQTT_WS,
  MQTT_WSS,
} transport_type_t;

/**
 * @brief Event ID for MQTT events
 */
typedef enum {
  MQTT_EVT_ANY = -1,
  MQTT_EVT_ERROR = 0,
  MQTT_EVT_CONNECTED,
  MQTT_EVT_DISCONNECTED,
  MQTT_EVT_SUBSCRIBED,
  MQTT_EVT_UNSUBSCRIBED,
  MQTT_EVT_PUBLISHED,
  MQTT_EVT_DATA,
  MQTT_EVT_BEFORE_CONNECT,
  MQTT_EVT_DELETED,
} mqtt_event_t;

/**
 * @brief Configuration structure for the MQTT client
 */
typedef struct mqtt_config {
  const char *host;
  const char *uri;
  const char *path;
  uint32_t port;
  const char *client_id;
  const char *username;
  const char *password;
  const char *lwt_topic;
  const char *lwt_msg;
  int lwt_qos;
  int lwt_retain;
  int lwt_msg_len;
  bool disable_clean_session;
  int keepalive;
  bool disable_auto_reconnect;
  int task_prio;
  int task_stack;
  int buffer_size;
  const char *server_cert_pem;
  const char *client_cert_pem;
  const char *client_key_pem;
  transport_type_t transport;
  mqtt_event_cb_fn_t event_cb_fn;
  void *user_context;
} mqtt_config_t;

/**
 * @brief MQTT event that will be received by the event callback function.
 */
typedef struct mqtt_event_data {
  int msg_id;
  char *topic;
  int topic_len;
  char *payload;
  int payload_len;
} mqtt_event_data_t;

/**
 * @brief Initialize MQTT client with mqtt broker ip address
 *
 * @param config the configuration of the MQTT client
 * @return mqtt_ctx_t* returns the pointer to the MQTT client context or NULL if failed
 */
mqtt_ctx_t *mqtt_client_init(mqtt_config_t *config);

/**
 * @brief Initialize MQTT client with mqtt broker uri
 *
 * @param config the configuration of the MQTT client
 * @return mqtt_ctx_t* returns the pointer to the MQTT client context or NULL if failed
 */
mqtt_ctx_t *mqtt_client_broker(mqtt_config_t *config);

/**
 * @brief Deinitialize MQTT client
 *
 * @param ctx the context of the MQTT client
 */
void mqtt_client_deinit(mqtt_ctx_t *ctx);

/**
 * @brief Start MQTT client
 *
 * @param ctx the context of the MQTT client
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_start(mqtt_ctx_t *ctx);

/**
 * @brief Stop MQTT client
 *
 * @param ctx the context of the MQTT client
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_stop(mqtt_ctx_t *ctx);

/**
 * @brief Subscribe to a topic
 *
 * @param ctx the context of the MQTT client
 * @param topic the topic to subscribe to
 * @param qos the quality of service to subscribe with
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_subscribe(mqtt_ctx_t *ctx, const char *topic, uint8_t qos);

/**
 * @brief Unsubscribe from a topic
 *
 * @param ctx the context of the MQTT client
 * @param topic the topic to unsubscribe from
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_unsubscribe(mqtt_ctx_t *ctx, const char *topic);

/**
 * @brief Publish a topic with payload
 *
 * @param ctx the context of the MQTT client
 * @param topic the topic to publish to
 * @param data the payload to publish
 * @param len the payload length
 * @param qos the quality of service to publish with
 * @param retain the retain flag to publish with
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_publish(mqtt_ctx_t *ctx, const char *topic, const char *data, uint16_t len, uint8_t qos,
                        uint8_t retain);

/**
 * @brief Re-connect to the MQTT broker
 *
 * @param ctx the context of the MQTT client
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_reconnect(mqtt_ctx_t *ctx);

/**
 * @brief Disconnect from the MQTT broker
 *
 * @param ctx the context of the MQTT client
 * @return int returns 0 if successful or -1 if failed
 */
int mqtt_client_disconnect(mqtt_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_H_ */
