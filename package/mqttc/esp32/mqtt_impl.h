/**
 * @file mqtt_impl.h
 *
 * @brief Wrapper API for ESP32 MQTT, the functions below are used in generic MQTT API.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef _MQTT_IMPL_H_
#define _MQTT_IMPL_H_

#include <stdint.h>

#include "mqtt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MQTT client instance that will be used by the MQTT client
 * It includes the MQTT configuration and client handle and the event callback function
 */
typedef struct mqtt_client mqtt_client_t;

/**
 * @brief Implement to create a new MQTT client with mqtt broker host ip address
 *
 * @param config Configuration structure for the MQTT client
 * @return mqtt_client_t* returns a pointer to the MQTT client instance on success, NULL on failure
 */
mqtt_client_t *mqtt_client_init_impl(mqtt_config_t *config);

/**
 * @brief Implement to create a new MQTT client with mqtt broker uri
 *
 * @param config Configuration structure for the MQTT client
 * @return mqtt_client_t* returns a pointer to the MQTT client instance on success, NULL on failure
 */
mqtt_client_t *mqtt_client_broker_uri_impl(mqtt_config_t *config);

/**
 * @brief Implement to destroy an MQTT client
 *
 * @param client the pointer to the MQTT client instance
 */
void mqtt_client_deinit_impl(mqtt_client_t *client);

/**
 * @brief Implement to register a callback function to receive MQTT events
 *
 * @param client the pointer to the MQTT client instance
 * @param callback the callback routine to receive MQTT events
 * @param handler_args the pointer to the handler arguments
 */
void mqtt_client_register_event_callback_impl(mqtt_client_t *client, mqtt_event_cb_fn_t callback, void *handler_args);

/**
 * @brief Implement to start an MQTT client
 *
 * @param client the pointer to the MQTT client instance
 * @return int returns 0 on success, -1 on failure
 */
int mqtt_client_start_impl(mqtt_client_t *client);

/**
 * @brief Implement to stop an MQTT client
 *
 * @param client the pointer to the MQTT client instance
 * @return int returns 0 on success, -1 on failure
 */
int mqtt_client_stop_impl(mqtt_client_t *client);

/**
 * @brief Implement to subscribe a message to an MQTT topic
 *
 * @param client the pointer to the MQTT client instance
 * @param topic the topic to subscribe to
 * @param qos the quality of service to subscribe with
 * @return int returns msg_id on success, -1 on failure
 *         msg_id is the message ID of the subscribe request
 */
int mqtt_client_subscribe_impl(mqtt_client_t *client, const char *topic, uint8_t qos);

/**
 * @brief Implement to unsubscribe a message from an MQTT topic
 *
 * @param client the pointer to the MQTT client instance
 * @param topic the topic to unsubscribe from
 * @return int returns msg_id on success, -1 on failure
 *         msg_id is the message ID of the subscribe request
 */
int mqtt_client_unsubscribe_impl(mqtt_client_t *client, const char *topic);

/**
 * @brief Implement to publish a message to an MQTT topic
 *
 * @param client the pointer to the MQTT client instance
 * @param topic the topic to publish to
 * @param payload the payload to publish
 * @param len the length of the payload
 * @param qos the quality of service to publish with
 * @param retain the retain flag to publish with
 * @return int returns msg_id on success, -1 on failure
 *         msg_id is the message id of the PUBLISH packet if qos != 0
 *         msg_id will always be zero(0) if qos == 0
 */
int mqtt_client_publish_impl(mqtt_client_t *client, const char *topic, const char *payload, uint16_t len, uint8_t qos,
                             uint8_t retain);

/**
 * @brief Implement to reconnect an MQTT client
 *
 * @param client the pointer to the MQTT client instance
 * @return int returns 0 on success, -1 on failure
 */
int mqtt_client_reconnect_impl(mqtt_client_t *client);

/**
 * @brief Implement to disconnect an MQTT client
 *
 * @param client the pointer to the MQTT client instance
 * @return int returns 0 on success, -1 on failure
 */
int mqtt_client_disconnect_impl(mqtt_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_IMPL_H_ */
