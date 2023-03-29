#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "cJSON.h"
#include "mqtt.h"
#include "log.h"
#include "sys_config.h"
#include "sys_status.h"
#include "espnow.h"
#include "comm_packet.h"

#include "mqtt_task.h"

SemaphoreHandle_t pub_mutex;
SemaphoreHandle_t msg_mutex;

static TaskHandle_t mqtt_task_handle;
static QueueHandle_t mqtt_msg_queue;

static mqtt_ctx_t *mqtt_ctx;

static const char *TAG = "MQTT";

static char json_data[1024];

#define MQTT_MSG_QUEUE_LEN 16

extern char main_mac_address[];

extern void send_msg_to_cmd_task(void *msg, size_t msg_len);

static time_t generate_time(int hour, int min) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  timeinfo.tm_hour = hour;
  timeinfo.tm_min = min;

  LOGI(TAG, "Year:%d,Mon:%d,Hour:%d,Min:%d,Sec:%d", timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_hour,
       timeinfo.tm_min, timeinfo.tm_sec);

  return mktime(&timeinfo);
}

static time_t get_current_time(void) {
  time_t now;
  struct tm timeinfo = { 0 };

  time(&now);
  localtime_r(&now, &timeinfo);

  return mktime(&timeinfo);
}

/*
 * Run command after parsing the command payload which comes from the App
 * Convert it to the espnow command to control valve device
 *
 */
void command_handler(cmd_t cmd, cJSON *payload) {
  /*
   *  {
   *    "cmd": "set" or "get",
   *    "prop": "devices", "list", "config", "stop", "add_device", "remove_device"
   *    "data": {}
   *  }
   *
   */
  irrigation_message_t msg;
  memset(&msg, 0x00, sizeof(irrigation_message_t));

  msg.current_time = get_current_time();

  switch (cmd) {
    case GET_DEVICE_LIST: {
      mqtt_response_t mqtt_resp = { 0 };
      mqtt_resp.resp = RESP_DEVICE_LIST;
      send_msg_to_mqtt_task(MQTT_PUB_DATA_ID, &mqtt_resp, sizeof(mqtt_response_t));
    } break;
    case SET_CONFIG_DATA: {
      mqtt_response_t mqtt_resp = { 0 };
      msg.sender_type = SET_CONFIG;
      cJSON *data = cJSON_GetObjectItemCaseSensitive(payload, PUBSUB_K_DATA);
      if (data && cJSON_IsObject(data)) {
        cJSON *start_time = cJSON_GetObjectItemCaseSensitive(data, PUBSUB_K_DATA_START_TIME);
        if (start_time && cJSON_IsObject(start_time)) {
          cJSON *hours = cJSON_GetObjectItemCaseSensitive(start_time, PUBSUB_K_DATA_HOURS);
          cJSON *minutes = cJSON_GetObjectItemCaseSensitive(start_time, PUBSUB_K_DATA_MINUTES);
          msg.payload.config.start_time = generate_time(hours->valueint, minutes->valueint);
        }
        cJSON *flow_rate = cJSON_GetObjectItemCaseSensitive(data, PUBSUB_K_DATA_FLOW_RATE);
        if (flow_rate) {
          cJSON *zones = cJSON_GetObjectItem(data, PUBSUB_K_DATA_ZONES);
          if (zones && cJSON_IsArray(zones)) {
            int zone_cnt = cJSON_GetArraySize(zones);
            msg.payload.config.zone_cnt = zone_cnt;
            for (int i = 0; i < zone_cnt; i++) {
              msg.payload.config.zones[i] = cJSON_GetArrayItem(zones, i)->valueint;
              msg.payload.config.flow_rate[i] = flow_rate->valueint;
            }
          }
        }
      }
      send_msg_to_cmd_task((void *)&msg, sizeof(irrigation_message_t));
      // Response payload of config to the broker server
      mqtt_resp.resp = RESP_CONFIG_DATA;
      send_msg_to_mqtt_task(MQTT_PUB_DATA_ID, &mqtt_resp, sizeof(mqtt_response_t));
    } break;
    case SET_FORCE_STOP: {
      msg.sender_type = FORCE_STOP;
      send_msg_to_cmd_task((void *)&msg, sizeof(irrigation_message_t));
    } break;
    case SET_UPDATE_DEVICE: {
      int id = 0;
      msg.sender_type = UPDATE_DEVICE_ADDR;
      device_manage_t *dev_manage = &msg.payload.dev_manage;
      cJSON *data = cJSON_GetObjectItemCaseSensitive(payload, PUBSUB_K_DATA);
      cJSON *devices = cJSON_GetObjectItemCaseSensitive(data, PUBSUB_K_DATA_DEVICES);
      if (data && devices && cJSON_IsArray(devices)) {
        cJSON *zone;
        cJSON_ArrayForEach(zone, devices) {
          cJSON *zone_id = cJSON_GetObjectItemCaseSensitive(zone, PUBSUB_K_DATA_ZONE_ID);
          cJSON *mac_address = cJSON_GetObjectItemCaseSensitive(zone, PUBSUB_K_DATA_MAC_ADDRESS);
          if (cJSON_IsNumber(zone_id) && cJSON_IsString(mac_address)) {
            memcpy(&dev_manage->update_dev_addr[id].mac_addr, mac_address->valuestring, (MAC_ADDR_LEN * 2));
            dev_manage->update_dev_addr[id].device_type = (device_type_t)zone_id->valueint;
            id++;
            LOGI(TAG, "zone_id = %d", (int)zone_id->valueint);
          }
        }
        dev_manage->update_dev_cnt = id;
      }
      send_msg_to_cmd_task((void *)&msg, sizeof(irrigation_message_t));
    } break;
    default: break;
  }
}

/*
 * Should be freed the json data that returned by callee in caller function.
 */
static char *generate_status_payload(response_t resp, void *data) {
  /*
   *  {
   *    "type": "response",
   *    "prop": "list", "config", "time_sync", "start", "stop",
   *            "done", "valve_on", "valve_off", "add_device", "remove_device"
   *    "data": {}
   *  }
   *
   */
  char *key[6] = { SYSCFG_N_CHILD1_MAC, SYSCFG_N_CHILD2_MAC, SYSCFG_N_CHILD3_MAC,
                   SYSCFG_N_CHILD4_MAC, SYSCFG_N_CHILD5_MAC, SYSCFG_N_CHILD6_MAC };

  char *output = NULL;
  payload_t *pd = (payload_t *)data;

  cJSON *payload = cJSON_CreateObject();

  switch (resp) {
    case RESP_DEVICE_LIST: {
      LOGI(TAG, "response device list!!!");
      cJSON_AddStringToObject(payload, PUBSUB_K_STATUS, PUBSUB_V_STATUS_RESPONSE);
      cJSON_AddStringToObject(payload, PUBSUB_K_PROP, PUBSUB_V_PROP_DEVICES);
      cJSON *data = cJSON_AddObjectToObject(payload, PUBSUB_K_DATA);
      cJSON *device_list = cJSON_CreateArray();

      int device_cnt = 0;
      char mac_addr[13] = { 0 };
      for (int i = 0; i < 6; i++) {
        memset(mac_addr, 0x00, sizeof(mac_addr));
        if (0 == syscfg_get(MFG_DATA, key[i], mac_addr, sizeof(mac_addr))) {
          LOGI(TAG, "%s = %s", key[i], mac_addr);
          cJSON *device = cJSON_CreateObject();
          cJSON_AddNumberToObject(device, PUBSUB_K_DATA_ZONE_ID, i + 1);
          cJSON_AddStringToObject(device, PUBSUB_K_DATA_MAC_ADDRESS, mac_addr);
          cJSON_AddItemToArray(device_list, device);
          device_cnt++;
        }
      }
      cJSON_AddStringToObject(data, PUBSUB_K_DATA_RESULT, PUBSUB_V_DATA_SUCCESS);
      cJSON_AddNumberToObject(data, "count", device_cnt);
      cJSON_AddItemToObject(data, "device_list", device_list);
      cJSON_AddItemToObject(data, PUBSUB_K_DATA_TIMESTAMP, cJSON_CreateString(log_timestamp()));

      output = cJSON_Print(payload);
    } break;
    case RESP_CONFIG_DATA: {
      cJSON_AddStringToObject(payload, PUBSUB_K_STATUS, PUBSUB_V_STATUS_RESPONSE);
      cJSON_AddStringToObject(payload, PUBSUB_K_PROP, PUBSUB_V_PROP_CONFIG);
      cJSON *data = cJSON_AddObjectToObject(payload, PUBSUB_K_DATA);
      cJSON_AddStringToObject(data, PUBSUB_K_DATA_RESULT, PUBSUB_V_DATA_SUCCESS);
      cJSON_AddItemToObject(data, PUBSUB_K_DATA_TIMESTAMP, cJSON_CreateString(log_timestamp()));

      output = cJSON_Print(payload);
    } break;
    case RESP_FLOW_START: {
      cJSON_AddStringToObject(payload, PUBSUB_K_STATUS, PUBSUB_V_STATUS_RESPONSE);
      cJSON_AddStringToObject(payload, PUBSUB_K_PROP, PUBSUB_V_PROP_START);
      cJSON *data = cJSON_AddObjectToObject(payload, PUBSUB_K_DATA);
      cJSON_AddStringToObject(data, PUBSUB_K_DATA_RESULT, PUBSUB_V_DATA_SUCCESS);
      cJSON_AddNumberToObject(data, PUBSUB_K_DATA_ZONE_ID, pd->dev_stat.deviceId);
      cJSON_AddItemToObject(data, PUBSUB_K_DATA_TIMESTAMP, cJSON_CreateString(log_timestamp()));

      output = cJSON_Print(payload);
    } break;
    case RESP_FORCE_STOP: {
      cJSON_AddStringToObject(payload, PUBSUB_K_STATUS, PUBSUB_V_STATUS_RESPONSE);
      cJSON_AddStringToObject(payload, PUBSUB_K_PROP, PUBSUB_V_PROP_STOP);
      cJSON *data = cJSON_AddObjectToObject(payload, PUBSUB_K_DATA);
      cJSON_AddStringToObject(data, PUBSUB_K_DATA_RESULT, PUBSUB_V_DATA_SUCCESS);
      cJSON_AddNumberToObject(data, PUBSUB_K_DATA_ZONE_ID, pd->dev_stat.deviceId);
      cJSON_AddNumberToObject(data, PUBSUB_K_DATA_FLOW_RATE, pd->dev_stat.flow_value);
      cJSON_AddItemToObject(data, PUBSUB_K_DATA_TIMESTAMP, cJSON_CreateString(log_timestamp()));

      output = cJSON_Print(payload);
    } break;
    case RESP_FLOW_DONE: {
      cJSON_AddStringToObject(payload, PUBSUB_K_STATUS, PUBSUB_V_STATUS_RESPONSE);
      cJSON_AddStringToObject(payload, PUBSUB_K_PROP, PUBSUB_V_PROP_DONE);
      cJSON *data = cJSON_AddObjectToObject(payload, PUBSUB_K_DATA);
      cJSON_AddStringToObject(data, PUBSUB_K_DATA_RESULT, PUBSUB_V_DATA_SUCCESS);
      cJSON_AddNumberToObject(data, PUBSUB_K_DATA_ZONE_ID, pd->dev_stat.deviceId);
      cJSON_AddNumberToObject(data, PUBSUB_K_DATA_FLOW_RATE, pd->dev_stat.flow_value);
      cJSON_AddItemToObject(data, PUBSUB_K_DATA_TIMESTAMP, cJSON_CreateString(log_timestamp()));

      output = cJSON_Print(payload);
    } break;
    case RESP_FLOW_ALL_DONE: {
    } break;
    case RESP_UPDATE_DEVICE: {
    } break;
    default: break;
  }

  // free resource
  if (output) {
    int len = strlen(output);

    memset(json_data, 0x00, sizeof(json_data));
    memcpy(json_data, output, len);

    free(output);
  }

  cJSON_Delete(payload);

  return json_data;
}

static int mqtt_reconnect(void) {
  LOGI(TAG, "Reconnect to the MQTT server!!!");
  return mqtt_client_reconnect(mqtt_ctx);
}

static int mqtt_publish(const char *topic, const char *payload, int qos, int retain) {
  int msg_id = -1;

  if (is_mqtt_init_finished() && is_mqtt_connected()) {
    if ((pub_mutex != NULL) && (xSemaphoreTake(pub_mutex, 2 * MQTT_FLAG_TIMEOUT) == pdTRUE)) {
      set_mqtt_published(0);
      msg_id = mqtt_client_publish(mqtt_ctx, topic, payload, strlen(payload), qos, retain);
      if (qos == QOS_0) {
        LOGI(TAG, "published qos0 data, topic: %s", topic);
      } else if (msg_id > 0) {
        LOGI(TAG, "published qos1 data, msg_id=%d, topic=%s", msg_id, topic);
      } else {
        LOGW(TAG, "failed to publish qos1, msg_id=%d", msg_id);
      }
      xSemaphoreGive(pub_mutex);
    } else {
      LOGW(TAG, "Cannot get mqtt semaphore!!!");
    }
  } else {
    LOGW(TAG, "mqtt connection may be problem...");
  }
  return msg_id;
}

static int mqtt_subscribe(const char *topic, int qos) {
  int msg_id = mqtt_client_subscribe(mqtt_ctx, topic, qos);
  return msg_id;
}

static int mqtt_req_cmd_handler(mqtt_topic_payload_t *p_mqtt) {
  int ret = 0;
  cJSON *mqtt_data = NULL;
  cmd_t req_cmd = NO_CMD;

  if (p_mqtt == NULL) {
    return -1;
  }

  if (p_mqtt->payload && p_mqtt->payload_len) {
    mqtt_data = cJSON_Parse(p_mqtt->payload);
    if (!mqtt_data) {
      LOGE(TAG, "Failed to parse mqtt data");
      ret = -1;
      goto handler_exit;
    }
  }

  // print pretty data of mqtt request
  if (mqtt_data) {
    char *mqtt_data_pretty = cJSON_Print(mqtt_data);
    if (p_mqtt->topic) {
      LOGI(TAG, "\nRequest topic data from broker:\n");
      LOGI(TAG, "topic = %s", p_mqtt->topic);
      LOGI(TAG, "payload = %s", mqtt_data_pretty);
    }
    free(mqtt_data_pretty);
  }

  // Parse command and payload data which comes from the App
  // Then, convert the command to the espnow command for hanlding valve devices.
  cJSON *cmd = cJSON_GetObjectItem(mqtt_data, PUBSUB_K_CMD);
  cJSON *prop = cJSON_GetObjectItem(mqtt_data, PUBSUB_K_PROP);

  if (!strcmp(cmd->valuestring, PUBSUB_V_CMD_SET)) {
    // Set Command
    if (!strcmp(prop->valuestring, PUBSUB_V_PROP_CONFIG)) {
      req_cmd = SET_CONFIG_DATA;
    } else if (!strcmp(prop->valuestring, PUBSUB_V_PROP_UPDATE_DEVICE)) {
      req_cmd = SET_UPDATE_DEVICE;
    } else if (!strcmp(prop->valuestring, PUBSUB_V_PROP_STOP)) {
      req_cmd = SET_FORCE_STOP;
    }
  } else if (!strcmp(cmd->valuestring, PUBSUB_V_CMD_GET)) {
    // Get Command
    if (!strcmp(prop->valuestring, PUBSUB_V_PROP_DEVICES)) {
      req_cmd = GET_DEVICE_LIST;
    }
  }

  command_handler(req_cmd, mqtt_data);

handler_exit:
  if (p_mqtt->topic) {
    free(p_mqtt->topic);
  }
  if (p_mqtt->payload) {
    free(p_mqtt->payload);
  }
  free(p_mqtt);

  if (mqtt_data) {
    cJSON_Delete(mqtt_data);
  }

  return ret;
}

void mqtt_msg_handler(mqtt_msg_t *mqtt_msg) {
  if (mqtt_msg->id == MQTT_EVENT_ID) {
    // Handle the request command comes from the App.
    mqtt_topic_payload_t *p_mqtt = (mqtt_topic_payload_t *)mqtt_msg->data;
    int ret = mqtt_req_cmd_handler(p_mqtt);
    LOGI(TAG, "mqtt_req_cmd_handler : ret = %d", ret);
  } else if (mqtt_msg->id == MQTT_PUB_DATA_ID) {
    // Handle the publish command to be sent the mobile app.
    if (mqtt_msg->data && mqtt_msg->data_len) {
      mqtt_response_t *p_mqtt_resp = (mqtt_response_t *)mqtt_msg->data;
      mqtt_publish_status(p_mqtt_resp);
    }
  }
}

int send_msg_to_mqtt_task(mqtt_msg_id_t id, void *data, uint32_t len) {
  int rc = 0;
  mqtt_msg_t mqtt_msg;

  // MQTT EVENT ID
  mqtt_topic_payload_t *p_data = NULL;
  mqtt_topic_payload_t *p_mqtt = NULL;

  // MQTT PUB DATA ID
  mqtt_response_t *p_mqtt_resp = NULL;

  xSemaphoreTake(msg_mutex, portMAX_DELAY);

  memset(&mqtt_msg, 0x00, sizeof(mqtt_msg_t));

  if ((id == MQTT_EVENT_ID) && data && len) {
    p_mqtt = (mqtt_topic_payload_t *)calloc(1, sizeof(mqtt_topic_payload_t));
    p_data = (mqtt_topic_payload_t *)data;
    if (p_data->topic && p_data->topic_len) {
      p_mqtt->topic = (char *)malloc(p_data->topic_len + 1);
      memset(p_mqtt->topic, 0x00, p_mqtt->topic_len + 1);
      memcpy(p_mqtt->topic, p_data->topic, p_data->topic_len);
      p_mqtt->topic_len = p_data->topic_len;
    }
    if (p_data->payload && p_data->payload_len) {
      p_mqtt->payload = (char *)malloc(p_data->payload_len + 1);
      memset(p_mqtt->payload, 0x00, p_mqtt->payload_len + 1);
      memcpy(p_mqtt->payload, p_data->payload, p_data->payload_len);
      p_mqtt->payload_len = p_data->payload_len;
    }
    mqtt_msg.id = id;
    mqtt_msg.data = p_mqtt;
    mqtt_msg.data_len = len;
  } else if (id == MQTT_PUB_DATA_ID) {
    mqtt_msg.id = id;
    if (len == sizeof(mqtt_response_t)) {
      p_mqtt_resp = (mqtt_response_t *)calloc(1, sizeof(mqtt_response_t));
      memcpy(p_mqtt_resp, (mqtt_response_t *)data, len);
      mqtt_msg.data = p_mqtt_resp;
      mqtt_msg.data_len = len;
    }
  }

  if (xQueueSendToBack(mqtt_msg_queue, &mqtt_msg, 0) != pdPASS) {
    if (id == MQTT_EVENT_ID && p_mqtt) {
      if (p_mqtt->topic) {
        free(p_mqtt->topic);
      }
      if (p_mqtt->payload) {
        free(p_mqtt->payload);
      }
      free(p_mqtt);
    } else if (id == MQTT_PUB_DATA_ID && p_mqtt_resp) {
      free(p_mqtt_resp);
    }
    rc = -1;
  }

  xSemaphoreGive(msg_mutex);
  return rc;
}

static void mqtt_event_callback(void *args, int32_t event_id, void *event_data) {
  mqtt_event_data_t *event = (mqtt_event_data_t *)event_data;
  mqtt_topic_payload_t mqtt;

  memset(&mqtt, 0x00, sizeof(mqtt));

  switch (event_id) {
    case MQTT_EVT_ERROR: LOGI(TAG, "MQTT Event Error!!!"); break;
    case MQTT_EVT_BEFORE_CONNECT: LOGI(TAG, "MQTT Event before connect!!!"); break;
    case MQTT_EVT_CONNECTED: {
      int msg_id = 0;
      char sub_topic[80] = { 0 };
      snprintf(sub_topic, sizeof(sub_topic), SUBSCRIBE_TOPIC, main_mac_address);
      LOGI(TAG, "sub_topic = %s", sub_topic);
      set_mqtt_connected(1);
      msg_id = mqtt_subscribe(sub_topic, 0);
      LOGI(TAG, "MQTT_EVT_CONNECTED!!!");
      LOGI(TAG, "mqtt_subscribe: msg_id = [%d]", msg_id);
      if (msg_id < 0) {
        mqtt_subscribe(sub_topic, 0);
      }
    } break;
    case MQTT_EVT_DISCONNECTED: {
      set_mqtt_connected(0);
      set_mqtt_published(0);
      set_mqtt_subscribed(0);
      LOGI(TAG, "MQTT_EVT_DISCONNECTED: Try to reconnect...");
      mqtt_reconnect();
    } break;
    case MQTT_EVT_SUBSCRIBED: {
      set_mqtt_subscribed(1);
      LOGI(TAG, "MQTT_EVT_SUBSCRIBED, msg_id = %d", event->msg_id);
    } break;
    case MQTT_EVT_UNSUBSCRIBED: LOGI(TAG, "MQTT_EVT_UNSUBSCRIBED"); break;
    case MQTT_EVT_PUBLISHED: {
      set_mqtt_published(1);
      LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id = %d", event->msg_id);
    } break;
    case MQTT_EVT_DATA: {
      LOGI(TAG, "MQTT_EVT_DATA");
      LOGI(TAG, "TOPIC = %.*s\n", event->topic_len, event->topic);
      LOGI(TAG, "PAYLOAD = %.*s\n", event->payload_len, event->payload);
      mqtt.topic = event->topic;
      mqtt.topic_len = event->topic_len;
      mqtt.payload = event->payload;
      mqtt.payload_len = event->payload_len;
      send_msg_to_mqtt_task(MQTT_EVENT_ID, &mqtt, sizeof(mqtt_topic_payload_t));
    } break;
    default: break;
  }
}

// External Functions

void mqtt_publish_status(mqtt_response_t *p_mqtt_resp) {
  char pub_topic[80] = { 0 };
  snprintf(pub_topic, sizeof(pub_topic), PUBLISH_TOPIC, main_mac_address);
  LOGI(TAG, "pub_topic = %s", pub_topic);
  mqtt_publish(pub_topic, generate_status_payload(p_mqtt_resp->resp, &p_mqtt_resp->payload), 0, 0);
  free(p_mqtt_resp);
}

int connect_mqtt_broker_host(const char *broker_addr, int port) {
  int ret = 0;

  mqtt_config_t config;
  memset(&config, 0x00, sizeof(mqtt_config_t));

  config.host = (const char *)broker_addr;
  config.port = port;
  config.transport = MQTT_TCP;
  config.event_cb_fn = mqtt_event_callback;

  mqtt_ctx = mqtt_client_init(&config);
  ret = mqtt_client_start(mqtt_ctx);
  if (ret) {
    LOGE(TAG, "Failed to connect to the mqtt server");
  }

  set_mqtt_init_finished(1);

  return ret;
}

int connect_mqtt_broker_uri(const char *broker_uri) {
  int ret = 0;

  mqtt_config_t config;
  memset(&config, 0x00, sizeof(mqtt_config_t));

  config.uri = broker_uri;
  config.event_cb_fn = mqtt_event_callback;

  mqtt_ctx = mqtt_client_broker(&config);
  ret = mqtt_client_start(mqtt_ctx);
  if (ret) {
    LOGE(TAG, "Failed to connect to the mqtt server");
  }

  set_mqtt_init_finished(1);

  return ret;
}

void disconnect_mqtt_broker(void) {
  if (mqtt_ctx) {
    mqtt_client_disconnect(mqtt_ctx);
    mqtt_client_deinit(mqtt_ctx);
    mqtt_ctx = NULL;
  }
}

static void mqtt_task(void *params) {
  mqtt_msg_t mqtt_msg;

  for (;;) {
    memset(&mqtt_msg, 0x00, sizeof(mqtt_msg_t));
    if (xQueueReceive(mqtt_msg_queue, &mqtt_msg, portMAX_DELAY) != pdPASS) {
      LOGE(TAG, "Cannot receive mqtt message from queue");
      vTaskDelay(1000);
      continue;
    }
    mqtt_msg_handler(&mqtt_msg);
  }
}

void create_mqtt_task(void) {
  pub_mutex = xSemaphoreCreateMutex();
  msg_mutex = xSemaphoreCreateMutex();

  mqtt_msg_queue = xQueueCreate(MQTT_MSG_QUEUE_LEN, sizeof(mqtt_msg_t));

  if (mqtt_msg_queue == NULL || pub_mutex == NULL || msg_mutex == NULL) {
    return;
  }

  xTaskCreatePinnedToCore(mqtt_task, MQTT_TASK_NAME, 4096, NULL, (tskIDLE_PRIORITY + 3), &mqtt_task_handle, 1);
}
