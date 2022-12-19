#ifndef _MQTT_TASK_H_
#define _MQTT_TASK_H_

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

/**
 * @brief
 *
 */
#if 0
typedef struct sens_pub_data {
} sens_pub_data_t;
#endif

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
  uint32_t len;
} mqtt_msg_t;

int start_mqttc(void);
void stop_mqttc(void);
void create_mqtt_task(void);
int send_msg_to_mqtt_task(mqtt_msg_id_t id, void *data, uint32_t len);
void mqtt_publish_sensor_data(void);

#ifdef __cplusplus
}
#endif

#endif /* _MQTT_TASK_H_ */
