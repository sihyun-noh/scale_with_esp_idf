/**
 * @file mqtt.c
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
#include "mqtt.h"
#include "esp32/mqtt_impl.h"

#include <stdio.h>
#include <string.h>

struct mqtt_ctx {
  mqtt_client_t *client;
  mqtt_event_cb_fn_t event_cb_fn;
};

mqtt_ctx_t *mqtt_client_init(mqtt_config_t *config) {
  mqtt_ctx_t *ctx = (mqtt_ctx_t *)malloc(sizeof(mqtt_ctx_t));
  if (ctx) {
    mqtt_client_t *client = mqtt_client_init_impl(config);
    ctx->client = client;
    ctx->event_cb_fn = config->event_cb_fn;
    if (ctx->event_cb_fn) {
      mqtt_client_register_event_callback_impl(ctx->client, ctx->event_cb_fn, ctx);
    }
  }
  return ctx;
}

mqtt_ctx_t *mqtt_client_broker(mqtt_config_t *config) {
  mqtt_ctx_t *ctx = (mqtt_ctx_t *)malloc(sizeof(mqtt_ctx_t));
  if (ctx) {
    mqtt_client_t *client = mqtt_client_broker_uri_impl(config);
    ctx->client = client;
    ctx->event_cb_fn = config->event_cb_fn;
    if (ctx->event_cb_fn) {
      mqtt_client_register_event_callback_impl(ctx->client, ctx->event_cb_fn, ctx);
    }
  }
  return ctx;
}

void mqtt_client_deinit(mqtt_ctx_t *ctx) {
  if (ctx) {
    mqtt_client_deinit_impl(ctx->client);
    free(ctx);
  }
}

int mqtt_client_start(mqtt_ctx_t *ctx) {
  if (!ctx) {
    return -1;
  }
  return mqtt_client_start_impl(ctx->client);
}

int mqtt_client_stop(mqtt_ctx_t *ctx) {
  if (!ctx) {
    return -1;
  }
  return mqtt_client_stop_impl(ctx->client);
}

int mqtt_client_subscribe(mqtt_ctx_t *ctx, const char *topic, uint8_t qos) {
  if (!ctx || !topic) {
    return -1;
  }
  return mqtt_client_subscribe_impl(ctx->client, topic, qos);
}

int mqtt_client_unsubscribe(mqtt_ctx_t *ctx, const char *topic) {
  if (!ctx || !topic) {
    return -1;
  }
  return mqtt_client_unsubscribe_impl(ctx->client, topic);
}

int mqtt_client_publish(mqtt_ctx_t *ctx, const char *topic, const char *data, uint16_t len, uint8_t qos,
                        uint8_t retain) {
  if (!ctx || !topic || !data) {
    return -1;
  }
  return mqtt_client_publish_impl(ctx->client, topic, data, len, qos, retain);
}

int mqtt_client_reconnect(mqtt_ctx_t *ctx) {
  if (!ctx) {
    return -1;
  }
  return mqtt_client_reconnect_impl(ctx->client);
}

int mqtt_client_disconnect(mqtt_ctx_t *ctx) {
  if (!ctx) {
    return -1;
  }
  return mqtt_client_disconnect_impl(ctx->client);
}
