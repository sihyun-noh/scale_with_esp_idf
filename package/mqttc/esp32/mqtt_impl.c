/**
 * @file mqtt_impl.c
 *
 * @brief Wrapper API for ESP32 MQTT, the functions below are used in generic
 MQTT API.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "mqtt_impl.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "esp_err.h"
#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt_impl";

struct mqtt_client {
  esp_mqtt_client_config_t config;
  esp_mqtt_client_handle_t handle;
  mqtt_event_cb_fn_t event_callback;
  void *event_callback_arg;
};

mqtt_event_data_t mqtt_event_data;

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
  mqtt_client_t *client = (mqtt_client_t *)handler_args;
  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

  mqtt_event_data.msg_id = event->msg_id;
  mqtt_event_data.topic = event->topic;
  mqtt_event_data.topic_len = event->topic_len;
  mqtt_event_data.payload = event->data;
  mqtt_event_data.payload_len = event->data_len;

  /* Call callback function if registered in upper layer API */
  client->event_callback(client->event_callback_arg, event_id, (void *)&mqtt_event_data);
}

static int apply_mqtt_config(esp_mqtt_client_config_t *esp_config, const mqtt_config_t *config) {
  bool is_secure = false;
  if (config->transport == MQTT_UNKNOWN) {
    ESP_LOGE(TAG, "Unknown transport type");
    return -1;
  }
  if (config->uri) {
    esp_config->broker.address.uri = config->uri;
  }
  if (config->host) {
    esp_config->broker.address.hostname = config->host;
  }
  if (config->port) {
    esp_config->broker.address.port = config->port;
  }
  if (config->transport) {
    esp_config->broker.address.transport = config->transport;
  }
  if (config->username) {
    esp_config->credentials.username = config->username;
  }
  if (config->password) {
    esp_config->credentials.authentication.password = config->password;
  }
  if (config->client_id) {
    esp_config->credentials.client_id = config->client_id;
  }
  if (config->transport == MQTT_SSL || config->transport == MQTT_WSS) {
    if (config->server_cert_pem) {
      esp_config->broker.verification.certificate = config->server_cert_pem;
      is_secure = true;
    }
    if (config->client_cert_pem) {
      esp_config->credentials.authentication.certificate = config->client_cert_pem;
      is_secure = true;
    }
    if (config->client_key_pem) {
      esp_config->credentials.authentication.key = config->client_key_pem;
      is_secure = true;
    }
    if (is_secure == false) {
      ESP_LOGE(TAG, "Should be used certificate file for MQTT SSL/WSS");
      return -1;
    }
  } else if (config->transport == MQTT_TCP || config->transport == MQTT_WS) {
    if (config->server_cert_pem || config->client_cert_pem || config->client_key_pem) {
      ESP_LOGE(TAG, "Invalid config for MQTT TCP/WS");
      return -1;
    }
  }
  return 0;
}

mqtt_client_t *mqtt_client_init_impl(mqtt_config_t *config) {
  mqtt_client_t *client = (mqtt_client_t *)malloc(sizeof(mqtt_client_t));
  if (client == NULL) {
    return NULL;
  }

  memset(client, 0, sizeof(mqtt_client_t));
  if (apply_mqtt_config(&client->config, config) != 0) {
    free(client);
    return NULL;
  }

  client->handle = esp_mqtt_client_init(&client->config);

  /* Register an internal mqtt event handler */
  esp_mqtt_client_register_event(client->handle, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  return client;
}

mqtt_client_t *mqtt_client_broker_uri_impl(mqtt_config_t *config) {
  mqtt_client_t *client = (mqtt_client_t *)malloc(sizeof(mqtt_client_t));
  if (client == NULL) {
    return NULL;
  }

  memset(client, 0, sizeof(mqtt_client_t));
  esp_mqtt_client_config_t mqtt_cfg = { 0 };

  ESP_LOGI(TAG, "broker uri = %s", config->uri);

  mqtt_cfg.broker.address.uri = config->uri;
  client->handle = esp_mqtt_client_init(&mqtt_cfg);

  /* Register an internal mqtt event handler */
  esp_mqtt_client_register_event(client->handle, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
  return client;
}

void mqtt_client_deinit_impl(mqtt_client_t *client) {
  if (client && client->handle) {
    esp_mqtt_client_destroy(client->handle);
  }
}

void mqtt_client_register_event_callback_impl(mqtt_client_t *client, mqtt_event_cb_fn_t callback, void *arg) {
  client->event_callback = callback;
  client->event_callback_arg = arg;
}

int mqtt_client_start_impl(mqtt_client_t *client) {
  if (client->handle == NULL) {
    return -1;
  }

  esp_err_t ret = esp_mqtt_client_start(client->handle);
  if (ret != ESP_OK) {
    return -1;
  }
  return 0;
}

int mqtt_client_stop_impl(mqtt_client_t *client) {
  if (client->handle == NULL) {
    return -1;
  }

  esp_err_t ret = esp_mqtt_client_stop(client->handle);
  if (ret != ESP_OK) {
    return -1;
  }
  return 0;
}

int mqtt_client_subscribe_impl(mqtt_client_t *client, const char *topic, uint8_t qos) {
  if (client->handle == NULL) {
    return -1;
  }

  return esp_mqtt_client_subscribe(client->handle, topic, qos);
}

int mqtt_client_publish_impl(mqtt_client_t *client, const char *topic, const char *payload, uint16_t len, uint8_t qos,
                             uint8_t retain) {
  if (client->handle == NULL) {
    return -1;
  }

  return esp_mqtt_client_publish(client->handle, topic, payload, len, qos, retain);
}

int mqtt_client_unsubscribe_impl(mqtt_client_t *client, const char *topic) {
  if (client->handle == NULL) {
    return -1;
  }

  return esp_mqtt_client_unsubscribe(client->handle, topic);
}

int mqtt_client_reconnect_impl(mqtt_client_t *client) {
  if (client->handle == NULL) {
    return -1;
  }

  esp_err_t ret = esp_mqtt_client_reconnect(client->handle);
  if (ret != ESP_OK) {
    return -1;
  }
  return 0;
}

int mqtt_client_disconnect_impl(mqtt_client_t *client) {
  if (client->handle == NULL) {
    return -1;
  }

  esp_err_t ret = esp_mqtt_client_disconnect(client->handle);
  if (ret != ESP_OK) {
    return -1;
  }
  return 0;
}
