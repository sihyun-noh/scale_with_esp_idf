#ifndef _MQTT_TASK_H_
#define _MQTT_TASK_H_

#include "comm_packet.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MQTT_TASK_NAME "MQTT_TASK"

#define MQTT_TIMEOUT 60
#define MQTT_FLAG_TIMEOUT (MQTT_TIMEOUT * 1000 / portTICK_PERIOD_MS)

#define QOS_0 0
#define QOS_1 1
#define NO_RETAIN 0
#define RETAIN 1

// topic name
#define PUBLISH_TOPIC "irrigation/%s/status"
#define SUBSCRIBE_TOPIC "irrigation/%s/cmd"

#define PUBSUB_K_CMD "cmd"
#define PUBSUB_V_CMD_SET "set"
#define PUBSUB_V_CMD_GET "get"

#define PUBSUB_K_PROP "prop"
#define PUBSUB_V_PROP_DEVICES "devices"
#define PUBSUB_V_PROP_TIME_SYNC "time_sync"
#define PUBSUB_V_PROP_CONFIG "config"
#define PUBSUB_V_PROP_START "start"
#define PUBSUB_V_PROP_STOP "stop"
#define PUBSUB_V_PROP_DONE "done"
#define PUBSUB_V_PROP_UPDATE_DEVICE "update_device"

#define PUBSUB_K_DATA "data"
#define PUBSUB_K_DATA_FLOW_RATE "flow_rate"
#define PUBSUB_K_DATA_ZONE_ID "zone_id"
#define PUBSUB_K_DATA_START_TIME "start_time"
#define PUBSUB_K_DATA_HOURS "hours"
#define PUBSUB_K_DATA_MINUTES "minutes"
#define PUBSUB_K_DATA_ZONES "zones"
#define PUBSUB_K_DATA_RESULT "result"
#define PUBSUB_K_DATA_COUNT "count"
#define PUBSUB_K_DATA_DEVICES "devices"
#define PUBSUB_K_DATA_MAC_ADDRESS "mac_address"
#define PUBSUB_K_DATA_TIMESTAMP "timestamp"

#define PUBSUB_K_STATUS "status"
#define PUBSUB_V_STATUS_RESPONSE "response"

#define PUBSUB_V_DATA_SUCCESS "success"
#define PUBSUB_V_DATA_FAILURE "failure"

typedef enum {
  NO_CMD = 0,
  GET_DEVICE_LIST = 1,
  SET_CONFIG_DATA = 2,
  SET_FORCE_STOP = 3,
  SET_UPDATE_DEVICE = 4,
} cmd_t;

typedef enum {
  RESP_DEVICE_LIST = 1,
  RESP_CONFIG_DATA,
  RESP_TIME_SYNC,
  RESP_FLOW_START,
  RESP_FLOW_DONE,
  RESP_FLOW_ALL_DONE,
  RESP_FORCE_STOP,
  RESP_VALVE_OFF,
  RESP_UPDATE_DEVICE,
} response_t;

typedef enum mqtt_msg_id {
  MQTT_EVENT_ID,
  MQTT_PUB_DATA_ID,
} mqtt_msg_id_t;

typedef struct mqtt_topic_payload {
  char *topic;
  uint32_t topic_len;
  char *payload;
  uint32_t payload_len;
} mqtt_topic_payload_t;

typedef struct mqtt_msg {
  mqtt_msg_id_t id;
  void *data;
  uint32_t data_len;
} mqtt_msg_t;

typedef struct mqtt_response {
  response_t resp;
  payload_t payload;
} mqtt_response_t;

int send_msg_to_mqtt_task(mqtt_msg_id_t id, void *data, uint32_t len);
void mqtt_publish_status(mqtt_response_t *p_mqtt_resp);
int connect_mqtt_broker_host(const char *broker_addr, int port);
int connect_mqtt_broker_uri(const char *broker_uri);
void disconnect_mqtt_broker(void);
void create_mqtt_task(void);

#ifdef __cplusplus
}
#endif

#endif  // _MQTT_TASK_H_
