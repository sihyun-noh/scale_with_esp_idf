#include <stdio.h>

#include "mqtt.h"

static mqtt_ctx_t *ctx;

static void mqtt_event_callback(void *handler_args, int32_t event_id, void *event_data) {
  // mqtt_ctx_t *ctx = (mqtt_ctx_t *)handler_args;
  mqtt_event_data_t *event = (mqtt_event_data_t *)event_data;

  printf("Event id: %ld\n", event_id);
  printf("event msg_id = %d\n", event->msg_id);
  switch (event_id) {
    case MQTT_EVT_ERROR: printf("event MQTT_EVT_ERROR\n"); break;
    case MQTT_EVT_CONNECTED: printf("event mqtt broker connected\n"); break;
    case MQTT_EVT_DISCONNECTED: printf("event mqtt broker disconnected\n"); break;
    case MQTT_EVT_SUBSCRIBED: printf("event mqtt subscribed\n"); break;
    case MQTT_EVT_DATA:
      printf("event MQTT_EVT_DATA\n");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("PAYLOAD=%.*s\r\n", event->payload_len, event->payload);
      break;
    default: break;
  }
}

int mqtt_start_cmd(int argc, char **argv) {
  if (argv[1] == NULL || argv[2] == NULL) {
    return -1;
  }

  char *host = argv[1];
  int port = atoi(argv[2]);
  mqtt_config_t config = {
    .transport = MQTT_TCP,
    .event_cb_fn = mqtt_event_callback,
    // .uri = "mqtt://mqtt.eclipseprojects.io",
    .host = host,
    .port = port,
  };

  ctx = mqtt_client_init(&config);

  return mqtt_client_start(ctx);
}

int mqtt_subscribe_cmd(int argc, char **argv) {
  if (argv[1] == NULL || argv[2] == NULL) {
    return -1;
  }

  char *topic = argv[1];
  int qos = atoi(argv[2]);

  int msg_id = mqtt_client_subscribe(ctx, topic, qos);
  printf("msg_id = %d\n", msg_id);
  return 0;
}

int mqtt_publish_cmd(int argc, char **argv) {
  if (argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) {
    return -1;
  }

  char *topic = argv[1];
  char *payload = argv[2];
  int qos = atoi(argv[3]);
  int len = 0;
  int retain = 0;

  int msg_id = mqtt_client_publish(ctx, topic, payload, len, qos, retain);
  printf("msg_id = %d\n", msg_id);
  return 0;
}

int mqtt_start(char *host, int port) {
  mqtt_config_t config = {
    .transport = MQTT_TCP,
    .event_cb_fn = mqtt_event_callback,
    // .uri = "mqtt://mqtt.eclipseprojects.io",
    .host = host,
    .port = port,
  };

  ctx = mqtt_client_init(&config);

  return mqtt_client_start(ctx);
}

int mqtt_publish(char *topic, char *payload, int qos) {
  int len = 0;
  int retain = 0;

  int msg_id = mqtt_client_publish(ctx, topic, payload, len, qos, retain);
  printf("msg_id = %d\n", msg_id);
  return 0;
}

int mqtt_subscribe(char *topic, int qos) {
  int msg_id = mqtt_client_subscribe(ctx, topic, qos);
  printf("msg_id = %d\n", msg_id);
  return 0;
}
