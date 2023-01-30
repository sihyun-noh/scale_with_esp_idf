#define USE_MQTTC

#include "cJSON.h"
#include "syslog.h"
#include "esp_wifi.h"
#include "sysevent.h"
#include "sys_config.h"
#include "sys_status.h"
#include "event_ids.h"
#if defined(USE_MQTTC)
#include "mqtt.h"
#elif defined(USE_LWMQTTC)
#include "mqttc.h"
#elif defined(USE_ASYNCMQTT)
#include "AsyncMqttClient.h"
#endif
#include "syscfg.h"
#include "filelog.h"
#include "ota_task.h"
#include "sysfile.h"
#include "mqtt_task.h"
#include "mqtt_topic.h"

#include "freertos/FreeRTOS.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
extern char *uptime(void);
extern void mqtt_publish_actuator_data(void);
}
#else
extern char *uptime(void);
extern void mqtt_publish_actuator_data(void);
#endif

SemaphoreHandle_t mqtt_semaphore;
static TaskHandle_t mqtt_task_handle;
static QueueHandle_t mqtt_task_msg_queue;

#if defined(USE_MQTTC)
static mqtt_ctx_t *mqtt_ctx;
#elif defined(USE_ASYNCMQTT)
AsyncMqttClient mqtt_client;
#endif

#define MQTT_MSG_QUEUE_LEN 16

static const char *TAG = "MQTT";

static char json_resp[1024];
static char json_data[1024];
static char json_fwup_resp[512];

uint8_t actuator_value[9] = { 0 };

char mqtt_request[128] = { 0 };
char mqtt_response[128] = { 0 };

static int minHeap = 0;
static void heap_monitor_func(int warning_level, int critical_level) {
  int freeHeap = xPortGetFreeHeapSize();
  if (minHeap == 0 || freeHeap < minHeap) {
    minHeap = freeHeap;
  }

  LOGI(TAG, "Free HeapSize = %d", freeHeap);
  LOGI(TAG, "uptime = %s", uptime());

  if (minHeap <= critical_level) {
    SLOGE(TAG, "Heap critical level reached: %d", critical_level);
  } else if (minHeap <= warning_level) {
    SLOGW(TAG, "Heap warning level reached: %d", warning_level);
  }
}

static int mqtt_reconnect(void) {
#if defined(USE_MQTTC)
  LOGI(TAG, "Reconnect to the MQTT server!!!");
  return mqtt_client_reconnect(mqtt_ctx);
#endif
}

static int mqtt_publish(char *topic, char *payload, int qos, int retain) {
  int msg_id = -1;

#if defined(USE_MQTTC)
  if (is_mqtt_init_finished() && is_mqtt_connected()) {
    if ((mqtt_semaphore != NULL) && xSemaphoreTake(mqtt_semaphore, 2 * MQTT_FLAG_TIMEOUT) == pdTRUE) {
      set_mqtt_published(0);
      msg_id = mqtt_client_publish(mqtt_ctx, topic, payload, strlen(payload), qos, retain);
      if (qos == QOS_0) {
        LOGI(TAG, "published qos0 data, topic: %s", topic);
      } else if (msg_id > 0) {
        LOGI(TAG, "published qos1 data, msg_id=%d, topic=%s", msg_id, topic);
        if (wait_for_sw_event(STATUS_MQTT_PUBLISHED, MQTT_FLAG_TIMEOUT)) {
          LOGI(TAG, "publish ack received, msg_id=%d", msg_id);
        } else {
          msg_id = mqtt_client_publish(mqtt_ctx, topic, payload, strlen(payload), qos, retain);
          if (msg_id < 0) {
            LOGW(TAG, "publish ack not received, msg_id=%d", msg_id);
          }
        }
      } else {
        msg_id = mqtt_client_publish(mqtt_ctx, topic, payload, strlen(payload), qos, retain);
        if (msg_id < 0) {
          LOGW(TAG, "failed to publish qos1, msg_id=%d", msg_id);
        }
      }
      xSemaphoreGive(mqtt_semaphore);
    } else {
      LOGW(TAG, "Cannot get mqtt semaphore!!!");
    }
  }
#elif defined(USE_LWMQTTC)
  if (is_mqtt_init_finished() && is_mqtt_connected()) {
    if ((mqtt_semaphore != NULL) && xSemaphoreTake(mqtt_semaphore, 2 * MQTT_FLAG_TIMEOUT) == pdTRUE) {
      set_mqtt_published(0);
      if (lwmqtt_client_publish((const char *)topic, (const uint8_t *)payload, strlen(payload), qos,
                                (retain == 1) ? true : false)) {
        LOGI(TAG, "published data, topic: %s", topic);
        msg_id = 0;
      } else {
        LOGW(TAG, "failed to publish, msg_id=%d", msg_id);
      }
      xSemaphoreGive(mqtt_semaphore);
    } else {
      LOGW(TAG, "Cannot get mqtt semaphore!!!");
    }
  }
#elif defined(USE_ASYNCMQTT)
  if (is_mqtt_init_finished() && is_mqtt_connected()) {
    if ((mqtt_semaphore != NULL) && xSemaphoreTake(mqtt_semaphore, 2 * MQTT_FLAG_TIMEOUT) == pdTRUE) {
      set_mqtt_published(0);
      msg_id = mqtt_client.publish((const char *)topic, 0, false, (const char *)payload, strlen(payload));
      if (msg_id > 0) {
        LOGI(TAG, "published data, msg_id=%d, topic=%s", msg_id, topic);
      } else {
        LOGW(TAG, "failed to publish, msg_id=%d", msg_id);
      }
      xSemaphoreGive(mqtt_semaphore);
    } else {
      LOGW(TAG, "Cannot get mqtt semaphore!!!");
    }
  }
#endif
  return msg_id;
}

static int mqtt_subscribe(char *topic, int qos) {
  int msg_id;
#if defined(USE_MQTTC)
  msg_id = mqtt_client_subscribe(mqtt_ctx, topic, qos);
  return msg_id;
#elif defined(USE_LWMQTTC)
  return lwmqtt_client_subscribe(topic, qos) == true ? 0 : -1;
#elif defined(USE_ASYNCMQTT)
  msg_id = mqtt_client.subscribe(topic, qos);
  return msg_id;
#endif
}

cJSON *gen_resp_obj(char *resp_type) {
  cJSON *root;
  char device_id[SYSCFG_S_DEVICEID] = { 0 };

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  root = cJSON_CreateObject();

  /* request type */
  cJSON_AddItemToObject(root, REQRES_K_TYPE, cJSON_CreateString(resp_type));
  /* devive id */
  cJSON_AddItemToObject(root, REQRES_K_DEVICEID, cJSON_CreateString(device_id));
  /* timestamp */
  cJSON_AddItemToObject(root, REQRES_K_TIMESTAMP, cJSON_CreateString(log_timestamp()));

  return root;
}

static char *gen_default_resp(char *resp_type) {
  /*
  {
    "type": "update", "reset"
    "id" : "GLSTH-0EADBEEFFEE9",
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  cJSON *root;
  char *output;

  root = gen_resp_obj(resp_type);

  output = cJSON_Print(root);
  memset(json_resp, 0x00, sizeof(json_resp));
  memcpy(json_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

#if (CONFIG_ACTUATOR_SWITCH)
static char *create_json_actuator_switch(void) {
  /*
  {
    "id" : "GLASW-0EADBEEFFEE9",
    "auto_mode" : "off",
    "value" : [{"io" : 1, "mode" : "on"},
                {"io" : 2, "mode" : "off"},
                {"io" : 3, "mode" : "on"},
                {"io" : 4, "mode" : "on"},
                {"io" : 5, "mode" : "on"},
                {"io" : 6, "mode" : "on"},
                {"io" : 7, "mode" : "on"},
                {"io" : 8, "mode" : "on"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  /* create root node and array */
  root = cJSON_CreateObject();

  /* devive id */
  cJSON_AddItemToObject(root, REQRES_K_DEVICEID, cJSON_CreateString(device_id));
  if (actuator_value[0] == 1) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("on"));
  } else if (actuator_value[0] == 0) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("off"));
  }

  cJSON *value = cJSON_CreateArray();
  cJSON_AddItemToObject(root, "value", value);
  for (uint8_t i = 0; i < 8; i++) {
    cJSON *io = cJSON_CreateObject();
    cJSON_AddItemToObject(io, "io", cJSON_CreateNumber(i + 1));
    if (actuator_value[i + 1] == 1) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("on"));
    } else if (actuator_value[i + 1] == 0) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("off"));
    }
    cJSON_AddItemToArray(value, io);
  }

  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));

  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}

#if 0
static char *create_json_actuator_switch_resp(uint8_t actuator_change[9]) {
  /*
  {
    "id" : "GLASW-0EADBEEFFEE9",
    "auto_mode" : "off",
    "value" : [{"io" : 1, "mode" : "on"},
                {"io" : 2, "mode" : "off"},
                {"io" : 3, "mode" : "on"},
                {"io" : 4, "mode" : "on"},
                {"io" : 5, "mode" : "on"},
                {"io" : 6, "mode" : "on"},
                {"io" : 7, "mode" : "on"},
                {"io" : 8, "mode" : "on"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;

  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("switch"));
  /* devive id */
  cJSON_AddItemToObject(root, REQRES_K_DEVICEID, cJSON_CreateString(device_id));
  if (actuator_value[0] == 1) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("on"));

    cJSON *value = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "value", value);
    for (uint8_t i = 0; i < 8; i++) {
      if (!actuator_change[i + 1]) {
        continue;
      }
      cJSON *io = cJSON_CreateObject();
      cJSON_AddItemToObject(io, "io", cJSON_CreateNumber(i + 1));
      if (actuator_value[i + 1] == 1) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("on"));
      } else if (actuator_value[i + 1] == 0) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("off"));
      }
      cJSON_AddItemToArray(value, io);
    }

  } else if (actuator_value[0] == 0) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("off"));
  }

  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));

  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}
#endif

static void actuator_switch_passing(cJSON *root) {
  // uint8_t actuator_change[9] = { 0 };
  /*
  {
    "type": "switch",
    "auto_mode" : "on",
    "value" : [ {"io" : 1, "mode" : "on"},
                {"io" : 2, "mode" : "off"}]
  }
  */

  cJSON *value = cJSON_GetObjectItem(root, "value");

  cJSON *auto_mode = cJSON_GetObjectItem(root, "auto_mode");

  if (cJSON_IsString(auto_mode)) {
    if (!strncmp(auto_mode->valuestring, "on", 2)) {
      int len = cJSON_GetArraySize(value);

      for (uint8_t i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(value, i);
        if (!item) {
          LOGI(TAG, "(): item %i is NULL, skipping.\n", i);
          continue;
        }
        cJSON *io = cJSON_GetObjectItem(item, "io");
        if (!io) {
          LOGI(TAG, "(): item %i has NULL io, skipping.\n", i);
          continue;
        }

        if (cJSON_IsNumber(io)) {
          if (io->valueint > 0 && io->valueint <= 8) {
            cJSON *mode = cJSON_GetObjectItem(item, "mode");
            if (!mode) {
              LOGI(TAG, "(): mode %i has NULL io, skipping.\n", i);
              continue;
            }
            if (cJSON_IsString(mode)) {
              if (!strncmp(mode->valuestring, "on", 2)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 1;
                // actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "off", 3)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 0;
                // actuator_change[io->valueint] = 1;
              }
            }
          }
        }
      }
      actuator_value[0] = 1;
      sysevent_set(ACTUATOR_AUTO, "on");
    } else if (!strncmp(auto_mode->valuestring, "off", 3)) {
      actuator_value[0] = 0;
      sysevent_set(ACTUATOR_AUTO, "off");
    }
  }
  // mqtt_publish(mqtt_response, create_json_actuator_switch_resp(actuator_change), ACTUATOR_QOS, 0);

  // cJSON_Delete(root);
}

#elif (CONFIG_ACTUATOR_MOTOR)
static char *create_json_actuator_motor(void) {
  /*
  {
    "id" : "GLAMT-0EADBEEFFEE9",
    "auto_mode" : "on",
    "value" : [ {"io" : 1, "mode" : "open"},
                {"io" : 2, "mode" : "open"},
                {"io" : 3, "mode" : "close"},
                {"io" : 4, "mode" : "stop"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;

  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  /* create root node and array */
  root = cJSON_CreateObject();

  /* devive id */
  cJSON_AddItemToObject(root, REQRES_K_DEVICEID, cJSON_CreateString(device_id));
  if (actuator_value[0] == 1) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("on"));
  } else if (actuator_value[0] == 0) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("off"));
  }

  cJSON *value = cJSON_CreateArray();
  cJSON_AddItemToObject(root, "value", value);
  for (uint8_t i = 0; i < 4; i++) {
    cJSON *io = cJSON_CreateObject();
    cJSON_AddItemToObject(io, "io", cJSON_CreateNumber(i + 1));
    if (actuator_value[i + 1] == 1) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("open"));
    } else if (actuator_value[i + 1] == 2) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("close"));
    } else if (actuator_value[i + 1] == 0) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("stop"));
    }
    cJSON_AddItemToArray(value, io);
  }

  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));

  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}
#if 0
static char *create_json_actuator_motor_resp(uint8_t actuator_change[9]) {
  /*
  {
    "id" : "GLAMT-0EADBEEFFEE9",
    "auto_mode" : "on",
    "value" : [{"io" : 1, "mode" : "open"},
                {"io" : 2, "mode" : "close"},
                {"io" : 3, "mode" : "close"},
                {"io" : 4, "mode" : "stop"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;

  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("motor"));
  /* devive id */
  cJSON_AddItemToObject(root, REQRES_K_DEVICEID, cJSON_CreateString(device_id));
  if (actuator_value[0] == 1) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("on"));

    cJSON *value = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "value", value);
    for (uint8_t i = 0; i < 4; i++) {
      if (!actuator_change[i + 1]) {
        continue;
      }
      cJSON *io = cJSON_CreateObject();
      cJSON_AddItemToObject(io, "io", cJSON_CreateNumber(i + 1));
      if (actuator_value[i + 1] == 1) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("open"));
      } else if (actuator_value[i + 1] == 2) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("close"));
      } else if (actuator_value[i + 1] == 0) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("stop"));
      }
      cJSON_AddItemToArray(value, io);
    }
  } else if (actuator_value[0] == 0) {
    cJSON_AddItemToObject(root, "auto_mode", cJSON_CreateString("off"));
  }

  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));

  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}
#endif
static void actuator_motor_passing(cJSON *root) {
  // uint8_t actuator_change[9] = { 0 };
  /*
  {
    "type": "motor",
    "auto_mode" : "on",
    "value" : [{"io" : 2, "mode" : "open"}]
  }
  */

  cJSON *value = cJSON_GetObjectItem(root, "value");

  cJSON *auto_mode = cJSON_GetObjectItem(root, "auto_mode");

  if (cJSON_IsString(auto_mode)) {
    if (!strncmp(auto_mode->valuestring, "on", 2)) {
      int len = cJSON_GetArraySize(value);

      for (uint8_t i = 0; i < len; i++) {
        cJSON *item = cJSON_GetArrayItem(value, i);
        if (!item) {
          LOGI(TAG, "(): item %i is NULL, skipping.\n", i);
          continue;
        }
        cJSON *io = cJSON_GetObjectItem(item, "io");
        if (!io) {
          LOGI(TAG, "(): item %i has NULL io, skipping.\n", i);
          continue;
        }

        if (cJSON_IsNumber(io)) {
          if (io->valueint > 0 && io->valueint <= 4) {
            cJSON *mode = cJSON_GetObjectItem(item, "mode");
            if (!mode) {
              LOGI(TAG, "(): mode %i has NULL io, skipping.\n", i);
              continue;
            }
            if (cJSON_IsString(mode)) {
              if (!strncmp(mode->valuestring, "open", 4)) {
                if (actuator_value[io->valueint] != 1) {
                  sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                  actuator_value[io->valueint] = 1;
                }
                // actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "close", 5)) {
                if (actuator_value[io->valueint] != 2) {
                  sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                  actuator_value[io->valueint] = 2;
                }
                // actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "stop", 4)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 0;
                // actuator_change[io->valueint] = 1;
              }
            }
          }
        }
      }
      actuator_value[0] = 1;
      sysevent_set(ACTUATOR_AUTO, "on");
    } else if (!strncmp(auto_mode->valuestring, "off", 3)) {
      actuator_value[0] = 0;
      sysevent_set(ACTUATOR_AUTO, "off");
    }
  }

  // mqtt_publish(mqtt_response, create_json_actuator_motor_resp(actuator_change), ACTUATOR_QOS, 0);

  // cJSON_Delete(root);
}
#endif

static char *gen_devinfo_resp(void) {
  /*
  {
    "type": "devinfo",
    "id":   "GLAMT-34945431DFEC",
    "timestamp":    "2022-11-08 16:50:25"
    "fw_version":   "ACTUATOR_SWITCH_1.0.1-188a61-DVT",
    "ip_address":   "192.168.50.22",
    "power":        "plug",
    "uptime":       "0:00:46",
    "free_mem":     "128436",
    "reconnect":    "1"
  }
  */

  cJSON *root;
  char *output;
  wifi_ap_record_t ap_info;

  esp_wifi_sta_get_ap_info(&ap_info);

  /* IP Addr assigned to Ethernet */
  esp_netif_ip_info_t ip_info;
  char ip_addr[16] = { 0 };
  char free_mem[20] = { 0 };
  char fw_version[SYSCFG_S_FWVERSION] = { 0 };
  char reconnect[SYSCFG_S_RECONNECT] = { 0 };

  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("ETH_DEF"), &ip_info);
  esp_ip4addr_ntoa(&ip_info.ip, ip_addr, sizeof(ip_addr));

  snprintf(free_mem, sizeof(free_mem), "%ld", xPortGetFreeHeapSize());
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  syscfg_get(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect, sizeof(reconnect));

  root = gen_resp_obj((char *)"devinfo");

  cJSON_AddItemToObject(root, REQRES_K_FWVERSION, cJSON_CreateString(fw_version));

  cJSON_AddItemToObject(root, REQRES_K_IPADDRESS, cJSON_CreateString((char *)ip_addr));
  if (!is_battery_model())
    cJSON_AddItemToObject(root, REQRES_K_POWERMODE, cJSON_CreateString("plug"));
  else
    cJSON_AddItemToObject(root, REQRES_K_POWERMODE, cJSON_CreateString("battery"));

  cJSON_AddItemToObject(root, REQRES_K_UPTIME, cJSON_CreateString(uptime()));
  cJSON_AddItemToObject(root, REQRES_K_FREEMEM, cJSON_CreateString(free_mem));
  cJSON_AddItemToObject(root, REQRES_K_RECONNECT, cJSON_CreateString(reconnect));

  output = cJSON_Print(root);

  memset(json_resp, 0x00, sizeof(json_resp));
  memcpy(json_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

static char *gen_fwupdate_resp(int result) {
  cJSON *root;
  char *output;
  char fw_version[SYSCFG_S_FWVERSION] = { 0 };

  root = gen_resp_obj((char *)"fw_update");

  if (result == 0) {
    cJSON_AddItemToObject(root, REQRES_K_STATE, cJSON_CreateString("success"));
  } else {
    cJSON_AddItemToObject(root, REQRES_K_STATE, cJSON_CreateString("failure"));
  }
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  cJSON_AddItemToObject(root, REQRES_K_FWVERSION, cJSON_CreateString(fw_version));

  // Print json payload
  output = cJSON_Print(root);
  memset(json_fwup_resp, 0x00, sizeof(json_fwup_resp));
  memcpy(json_fwup_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_fwup_resp);

  return json_fwup_resp;
}

static char *gen_syscfg_resp(cJSON *action, cJSON *result) {
  cJSON *root;
  char *output;

  root = gen_resp_obj((char *)"syscfg");
  cJSON_AddItemToObject(root, REQRES_K_ACTION, action);
  cJSON_AddItemToObject(root, REQRES_K_RESULT, result);

  output = cJSON_Print(root);
  memset(json_resp, 0x00, sizeof(json_resp));
  memcpy(json_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

static void devinfo_cmd_handler(cJSON *root) {
  mqtt_publish(mqtt_response, gen_devinfo_resp(), 0, 0);
}

static void update_sensor_data_cmd_handler(cJSON *root) {
  (void)root;

  mqtt_publish(mqtt_response, gen_default_resp((char *)"update"), 0, 0);

  mqtt_publish_actuator_data();
}

static void reset_device_cmd_handler(cJSON *root) {
  (void)root;

  mqtt_publish(mqtt_response, gen_default_resp((char *)"reset"), 0, 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void factory_reset_cmd_handler(cJSON *root) {
  (void)root;

  if (syscfg_clear((syscfg_type_t)0) != 0) {
    LOGE(TAG, "Failed to sys CFG_DATA clear");
  }
  mqtt_publish(mqtt_response, gen_default_resp((char *)"factory_reset"), 0, 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void wifi_change_cmd_handler(cJSON *root) {
  char farmssid[SYSCFG_S_SSID] = { 0 };
  char farmpw[SYSCFG_S_PASSWORD] = { 0 };

  cJSON *ssid = cJSON_GetObjectItem(root, REQRES_K_SSID);
  cJSON *pw = cJSON_GetObjectItem(root, REQRES_K_PASSWORD);
  // ssid pw
  if (cJSON_IsString(ssid) && cJSON_IsString(pw)) {
    snprintf(farmssid, sizeof(farmssid), "%s", ssid->valuestring);
    snprintf(farmpw, sizeof(farmpw), "%s", pw->valuestring);
    syscfg_set(SYSCFG_I_SSID, SYSCFG_N_SSID, farmssid);
    syscfg_set(SYSCFG_I_PASSWORD, SYSCFG_N_PASSWORD, farmpw);
    LOGI(TAG, "Got SSID = [%s] password = [%s]", farmssid, farmpw);
  }
  SLOGI(TAG, "Resetting...");
  mqtt_publish(mqtt_response, gen_default_resp((char *)"wifi_change"), 0, 0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void server_change_cmd_handler(cJSON *root) {
  char farmip[SYSCFG_S_FARMIP] = { 0 };
  cJSON *Server = cJSON_GetObjectItem(root, REQRES_K_SERVER);
  if (cJSON_IsString(Server)) {
    snprintf(farmip, sizeof(farmip), "%s", Server->valuestring);
    syscfg_set(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip);
    LOGI(TAG, "Got server url = %s ", Server->valuestring);
  }
  SLOGI(TAG, "Resetting...");
  mqtt_publish(mqtt_response, gen_default_resp((char *)"server_change"), 0, 0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void serach_device_cmd_handler(cJSON *root) {
  (void)root;

  set_identification(1);
  mqtt_publish(mqtt_response, gen_default_resp((char *)"search"), 0, 0);
}

static void spiffs_format_cmd_handler(cJSON *root) {
  (void)root;

  sysfile_format();
  mqtt_publish(mqtt_response, gen_default_resp((char *)"spiffs_format"), 0, 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void update_fwversion_cmd_handler(cJSON *root) {
  cJSON *url = cJSON_GetObjectItem(root, REQRES_K_URL);

  if (!url) {
    LOGE(TAG, "Failed to get firmware download URL from mqtt payload");
    return;
  }

  set_fwupdate(1);
  LOGI(TAG, "fw download url = %s", url->valuestring);

  int ret = start_ota_fw_task_wait(url->valuestring);
  LOGI(TAG, "start_ota_fw_task_wait : ret = %d", ret);

  mqtt_publish(mqtt_response, gen_fwupdate_resp(ret), 0, 0);
  set_fwupdate(0);
}

/* "syscfg set" action */
static int syscfg_set_action(char *name, char *value) {
  int index = get_syscfg_idx(name);
  if (index < 0) {
    return -1;
  }

  if (syscfg_set((syscfg_type_t)index, name, value) != 0) {
    return -1;
  }
  return 0;
}

/* "syscfg get" action */
static int syscfg_get_action(char *name, char *buff, uint32_t buff_size) {
  int index = get_syscfg_idx(name);
  if (index < 0) {
    return -1;
  }

  if (syscfg_get((syscfg_type_t)index, name, buff, buff_size) != 0) {
    return -1;
  }
  return 0;
}

/* "syscfg show" action */
static int syscfg_show_action(cJSON **object) {
  cJSON *syscfg_obj = dump_syscfg_to_json_object();
  if (!syscfg_obj) {
    return -1;
  }
  *object = syscfg_obj;
  return 0;
}

#define SYSCFG_OP_SIZE 16

static cJSON *syscfg_action(char *action) {
  cJSON *result = NULL;
  char op[SYSCFG_OP_SIZE + 1] = { 0 };
  char var_name[SYSCFG_VARIABLE_NAME_SIZE] = { 0 };
  char var_value[SYSCFG_VARIABLE_VALUE_SIZE] = { 0 };
  char *result_val = REQRES_V_ERR_SYSCFG_FAIL;

  /* Get operation type, name, value from action string */
  /* (e.g.) action : set power_mode P or get power_mode */
  sscanf(action, "%s%s%s", op, var_name, var_value);

  LOGI(TAG, "op = %s", op);
  LOGI(TAG, "var_name = %s", var_name);
  LOGI(TAG, "var_value = %s", var_value);

  /* "syscfg set" command */
  if (!strncmp(op, REQRES_V_SYSCFG_SET, strlen(REQRES_V_SYSCFG_SET))) {
    /* Execute "syscfg set action" */
    if (!syscfg_set_action(var_name, var_value)) {
      result_val = REQRES_V_SYSCFG_OK;
    }
    result = cJSON_CreateString(result_val);
  }
  /* "syscfg get" command */
  else if (!strncmp(op, REQRES_V_SYSCFG_GET, strlen(REQRES_V_SYSCFG_GET))) {
    memset(var_value, 0x00, sizeof(var_value));
    /* Execute "syscfg get action" */
    if (!syscfg_get_action(var_name, var_value, sizeof(var_value))) {
      result_val = var_value;
    }
    result = cJSON_CreateString(result_val);
  }
  /* "syscfg show" command */
  else if (!strncmp(op, REQRES_V_SYSCFG_SHOW, strlen(REQRES_V_SYSCFG_SHOW))) {
    cJSON *obj = NULL;
    /* Execute "syscfg show action" */
    if (!syscfg_show_action(&obj)) {
      result = cJSON_Duplicate(obj, 1);
      cJSON_Delete(obj);
    } else {
      result = cJSON_CreateString(REQRES_V_ERR_SYSCFG_NONE);
    }
  }

  return result;
}

static void syscfg_cmd_handler(cJSON *root) {
  cJSON *result;
  cJSON *action = cJSON_GetObjectItem(root, REQRES_K_ACTION);

  if (!action) {
    LOGE(TAG, "Failed to get action from mqtt payload");
    return;
  }

  result = syscfg_action(action->valuestring);

  // Use duplicate action json object instead of using action object directly, because it(action) is freed in mqtt
  // request handler.
  cJSON *resp_action = cJSON_Duplicate(action, 1);
  mqtt_publish(mqtt_response, gen_syscfg_resp(resp_action, result), 0, 0);
}

// static int mqtt_req_cmd_handler(cJSON *mqtt_data) {
static int mqtt_req_cmd_handler(mqtt_topic_payload_t *p_mqtt) {
  int ret = 0;
  cJSON *mqtt_data = NULL;

  if (!p_mqtt) {
    return -1;
  }

  if (p_mqtt->payload) {
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

  {
    cJSON *req_type = cJSON_GetObjectItem(mqtt_data, REQRES_K_TYPE);

    if (!req_type) {
      LOGE(TAG, "Failed to get req type from mqtt data");
      ret = -1;
      goto handler_exit;
    }

    /* devinfo */
    if (!strcmp(req_type->valuestring, REQRES_V_DEVINFO)) {
      devinfo_cmd_handler(mqtt_data);
    }
    /* fw_update */
    else if (!strcmp(req_type->valuestring, REQRES_V_FWUPDATE)) {
      update_fwversion_cmd_handler(mqtt_data);
    }
    /* update */
    else if (!strcmp(req_type->valuestring, REQRES_V_UPDATE)) {
      update_sensor_data_cmd_handler(mqtt_data);
    }
    /* reset a device */
    else if (!strcmp(req_type->valuestring, REQRES_V_RESET)) {
      reset_device_cmd_handler(mqtt_data);
    }
    /* factory reset */
    else if (!strcmp(req_type->valuestring, REQRES_V_FACTORY_RESET)) {
      factory_reset_cmd_handler(mqtt_data);
    }
    /* wifi change */
    else if (!strcmp(req_type->valuestring, REQRES_V_WIFI_CHANGE)) {
      wifi_change_cmd_handler(mqtt_data);
    }
    /* server change */
    else if (!strcmp(req_type->valuestring, REQRES_V_SERVER_CHANGE)) {
      server_change_cmd_handler(mqtt_data);
    }
    /* serach (identify) */
    else if (!strcmp(req_type->valuestring, REQRES_V_SEARCH)) {
      serach_device_cmd_handler(mqtt_data);
    }
    /* spiffs (file) format */
    else if (!strcmp(req_type->valuestring, REQRES_V_SPIFFS_FORMAT)) {
      spiffs_format_cmd_handler(mqtt_data);
    }
    /* syscfg action (set/get/show) */
    else if (!strcmp(req_type->valuestring, REQRES_V_SYSCFG)) {
      syscfg_cmd_handler(mqtt_data);
    }
#if (CONFIG_ACTUATOR_SWITCH)
    /* actuator switch control command */
    else if (!strcmp(req_type->valuestring, REQRES_V_SWITCH)) {
      actuator_switch_passing(mqtt_data);
      mqtt_publish_actuator_data();
    }
#endif
#if (CONFIG_ACTUATOR_MOTOR)
    /* actuator motor control command */
    else if (!strcmp(req_type->valuestring, REQRES_V_MOTOR)) {
      actuator_motor_passing(mqtt_data);
      mqtt_publish_actuator_data();
    }
#endif
  }

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

// MQTT EVENT MSG (REQUESST) HANDLER
void mqtt_msg_handler(mqtt_msg_t *mqtt_msg) {
  mqtt_msg_t *msg = mqtt_msg;

  if (msg->id == MQTT_EVENT_ID) {
    mqtt_topic_payload_t *p_mqtt = (mqtt_topic_payload_t *)msg->data;
    /* Process the request command */
    int ret = mqtt_req_cmd_handler(p_mqtt);
    LOGI(TAG, "mqtt_req_cmd_handler : ret = %d", ret);
  } else if (msg->id == MQTT_PUB_DATA_ID) {
    LOGI(TAG, "mqtt_publish_actuator_data");
    mqtt_publish_actuator_data();
  }
}

#if defined(USE_MQTTC)

static void mqtt_event_callback(void *handler_args, int32_t event_id, void *event_data) {
  mqtt_event_data_t *event = (mqtt_event_data_t *)event_data;
  mqtt_topic_payload_t mqtt = {
    0,
  };

  switch (event_id) {
    case MQTT_EVT_ERROR: LOGI(TAG, "MQTT_EVT_ERROR !!!"); break;
    case MQTT_EVT_BEFORE_CONNECT: LOGI(TAG, "MQTT_EVT_BEFORE_CONNECT !!!"); break;
    case MQTT_EVT_CONNECTED: {
      int msg_id = 0;
      set_mqtt_connected(1);
      msg_id = mqtt_subscribe(mqtt_request, 0);
      LOGI(TAG, "MQTT_EVT_CONNECTED !!!");
      LOGI(TAG, "mqtt_subscribe, msg_id = %d", msg_id);
      if (msg_id < 0) {
        mqtt_subscribe(mqtt_request, 0);
      }
    } break;
    case MQTT_EVT_DISCONNECTED:
      set_mqtt_connected(0);
      set_mqtt_published(0);
      set_mqtt_subscribed(0);
      LOGI(TAG, "MQTT_EVT_DISCONNECTED !!!");
      // TODO : Reconnect to the MQTT server (call mqtt_reconnect) when connection is lost
      mqtt_reconnect();
      break;
    case MQTT_EVT_SUBSCRIBED:
      set_mqtt_subscribed(1);
      LOGI(TAG, "MQTT_EVT_SUBSCRIBED, msg_id = %d", event->msg_id);
      break;
    case MQTT_EVT_UNSUBSCRIBED: LOGI(TAG, "MQTT_EVT_UNSUBSCRIBED, msg_id = %d", event->msg_id); break;
    case MQTT_EVT_PUBLISHED:
      set_mqtt_published(1);
      LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id = %d", event->msg_id);
      break;
    case MQTT_EVT_DATA:
      LOGI(TAG, "MQTT_EVT_DATA");
      LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
      LOGI(TAG, "PAYLOAD=%.*s\r\n", event->payload_len, event->payload);
      mqtt.topic = event->topic;
      mqtt.topic_len = event->topic_len;
      mqtt.payload = event->payload;
      mqtt.payload_len = event->payload_len;
      send_msg_to_mqtt_task(MQTT_EVENT_ID, &mqtt, sizeof(mqtt_topic_payload_t));
      break;
    default: break;
  }
}

#elif defined(USE_LWMQTTC)

#define MQTT_RESTART_INTERVAL (3600 * 1000)

#if 0
static void mqtt_task_restart(void *unused) {
  for (;;) {
    // Restart periodically
    vTaskDelay(MQTT_RESTART_INTERVAL / portTICK_PERIOD_MS);
    if (is_mqtt_init_finished()) {
      stop_mqttc();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if (!is_mqtt_init_finished()) {
      start_mqttc();
    }
  }
}
#endif

static void status_callback(esp_mqtt_status_t status) {
  switch (status) {
    case ESP_MQTT_STATUS_CONNECTED:
      // Subscribe
      set_mqtt_connected(1);
      mqtt_subscribe(mqtt_request, 0);
      LOGI(TAG, "MQTT_STATUS_CONNECTED !!!");
      break;
    case ESP_MQTT_STATUS_DISCONNECTED:
      set_mqtt_connected(0);
      set_mqtt_published(0);
      set_mqtt_subscribed(0);
      LOGI(TAG, "MQTT_STATUS_DISCONNECTED !!!");
      // TODO : Reconnect to the MQTT server (call mqtt_reconnect) when connection is lost
      break;
    default: break;
  }
}

static void message_callback(const char *topic, const uint8_t *payload, size_t len, int qos, bool retained) {
  mqtt_topic_payload_t mqtt = { 0 };

  mqtt.topic = topic;
  mqtt.topic_len = stelen(topic);
  mqtt.payload = payload;
  mqtt.payload_len = len;

  send_msg_to_mqtt_task(MQTT_EVENT_ID, &mqtt, sizeof(mqtt_topic_payload_t));

  LOGI(TAG, "Incoming: %s => %s (len = %d, qos = %d, retain = %d)", topic, payload, (int)len, qos, retained);
}

#elif defined(USE_ASYNCMQTT)

void mqtt_connect_callback(bool session_present) {
  set_mqtt_connected(1);
  mqtt_subscribe(mqtt_request, 0);
  LOGI(TAG, "Connect to MQTT server");
}

void mqtt_disconnect_callback(AsyncMqttClientDisconnectReason reason) {
  set_mqtt_connected(0);
  set_mqtt_published(0);
  set_mqtt_subscribed(0);
  LOGI(TAG, "Disconnect from MQTT server");
  // TODO : Reconnect to the MQTT server (call mqtt_reconnect) when connection is lost
}

void mqtt_subscribe_callback(uint16_t packet_id, uint8_t qos) {
  set_mqtt_subscribed(1);
  LOGI(TAG, "subscribed mqtt data : msg_id = %d", packet_id);
}

void mqtt_unsubscribe_callback(uint16_t packet_id) {}

void mqtt_message_callback(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len,
                           size_t index, size_t total) {
  mqtt_topic_payload_t mqtt = {
    0,
  };

  mqtt.topic = topic;
  mqtt.topic_len = strlen(topic);
  mqtt.payload = payload;
  mqtt.payload_len = len;

  send_msg_to_mqtt_task(MQTT_EVENT_ID, &mqtt, sizeof(mqtt_topic_payload_t));

  LOGI(TAG, "Incoming: %s => %s (len = %d)", topic, payload, (int)len);
}

void mqtt_publish_callback(uint16_t packet_id) {
  set_mqtt_published(1);
  LOGI(TAG, "published mqtt data : msg_id = %d", packet_id);
}

#endif

int start_mqttc(void) {
  int ret = 0;
  char farmip[SYSCFG_S_FARMIP] = { 0 };
  char device_id[SYSCFG_S_DEVICEID] = { 0 };

  syscfg_get(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip, sizeof(farmip));
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  snprintf(mqtt_request, sizeof(mqtt_request), CMD_REQUEST_TOPIC, device_id);
  snprintf(mqtt_response, sizeof(mqtt_response), CMD_RESPONSE_TOPIC, device_id);

  if (!farmip[0]) {
    return -1;
  }

  char *s_host = strtok(farmip, ":");  // 공백을 기준으로 문자열 자르기
  char *s_port = strtok(NULL, " ");    // 널문자를 기준으로 다시 자르기

  if (s_host == NULL || s_port == NULL) {
    return -1;
  }

  int n_port = atoi(s_port);

  LOGI(TAG, "Host IP address = %s, port = %d", s_host, n_port);

#if defined(USE_LWMQTTC)
  lwmqtt_client_start(s_host, s_port, "mqtt", NULL, NULL);
#elif defined(USE_MQTTC)
  mqtt_config_t config = {
    .host = (const char *)s_host,
    .port = (unsigned int)n_port,
    .transport = MQTT_TCP,
    .event_cb_fn = mqtt_event_callback,
  };

  mqtt_ctx = mqtt_client_init(&config);
  ret = mqtt_client_start(mqtt_ctx);
  if (ret != 0) {
    LOGE(TAG, "Failed to mqtt start");
  }
#elif defined(USE_ASYNCMQTT)
  IPAddress host_addr(192, 168, 50, 101);

  mqtt_client.setServer(host_addr, n_port);
  mqtt_client.connect();
#endif

  set_mqtt_init_finished(1);
  return ret;
}

void stop_mqttc(void) {
#if defined(USE_LWMQTTC)
  lwmqtt_client_stop();
#elif defined(USE_MQTTC)
  if (mqtt_ctx) {
    mqtt_client_disconnect(mqtt_ctx);
    mqtt_client_deinit(mqtt_ctx);
    mqtt_ctx = NULL;
  }
#elif defined(USE_ASYNCMQTT)
  mqtt_client.disconnect();
#endif
  set_mqtt_init_finished(0);
}

void mqtt_publish_actuator_data(void) {
  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  int ret = 0;

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

#if (CONFIG_ACTUATOR_SWITCH)
  char mqtt_switch[80] = { 0 };
  snprintf(mqtt_switch, sizeof(mqtt_switch), ACTUATOR_SWITCH_PUB_SUB_TOPIC, device_id);
  ret = mqtt_publish(mqtt_switch, create_json_actuator_switch(), ACTUATOR_QOS, 0);
  if (ret < 0) {
    LOGI(TAG, "mqtt_publish error!");
  }
#elif (CONFIG_ACTUATOR_MOTOR)
  char mqtt_motor[80] = { 0 };
  snprintf(mqtt_motor, sizeof(mqtt_motor), ACTUATOR_MOTOR_PUB_SUB_TOPIC, device_id);
  ret = mqtt_publish(mqtt_motor, create_json_actuator_motor(), ACTUATOR_QOS, 0);
  if (ret < 0) {
    LOGI(TAG, "mqtt_publish error!");
  }
#endif
  heap_monitor_func(8092, 4096);
}

int send_msg_to_mqtt_task(mqtt_msg_id_t id, void *data, uint32_t len) {
  mqtt_msg_t mqtt_msg;
  mqtt_topic_payload_t *p_data = NULL;
  mqtt_topic_payload_t *p_mqtt = NULL;

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
    mqtt_msg.len = len;
  } else if (id == MQTT_PUB_DATA_ID) {
    mqtt_msg.id = id;
    mqtt_msg.data = NULL;
    mqtt_msg.len = 0;
  }

  if (xQueueSendToBack(mqtt_task_msg_queue, &mqtt_msg, 0) != pdPASS) {
    if (id == MQTT_EVENT_ID && p_mqtt) {
      if (p_mqtt->topic) {
        free(p_mqtt->topic);
      }
      if (p_mqtt->payload) {
        free(p_mqtt->payload);
      }
      free(p_mqtt);
    }
    return -1;
  }

  return 0;
}

static void mqtt_task(void *params) {
  mqtt_msg_t mqtt_msg;

  for (;;) {
    memset(&mqtt_msg, 0x00, sizeof(mqtt_msg_t));
    if (xQueueReceive(mqtt_task_msg_queue, &mqtt_msg, portMAX_DELAY) != pdPASS) {
      LOGE(TAG, "Cannot receive mqtt message from queue");
      continue;
    }
    mqtt_msg_handler(&mqtt_msg);
  }
}

void create_mqtt_task(void) {
  mqtt_semaphore = xSemaphoreCreateMutex();
  mqtt_task_msg_queue = xQueueCreate(MQTT_MSG_QUEUE_LEN, sizeof(mqtt_msg_t));

  if (mqtt_task_msg_queue == NULL || mqtt_semaphore == NULL) {
    return;
  }

#if defined(USE_LWMQTTC)

  // Initialize mqtt
  lwmqtt_client_init(status_callback, message_callback, 256, 2000);

#elif defined(USE_ASYNCMQTT)

  mqtt_client.onConnect(mqtt_connect_callback);
  mqtt_client.onDisconnect(mqtt_disconnect_callback);
  mqtt_client.onSubscribe(mqtt_subscribe_callback);
  mqtt_client.onUnsubscribe(mqtt_unsubscribe_callback);
  mqtt_client.onMessage(mqtt_message_callback);
  mqtt_client.onPublish(mqtt_publish_callback);

#endif

  xTaskCreatePinnedToCore(mqtt_task, MQTT_TASK_NAME, 4096, NULL, (tskIDLE_PRIORITY + 5),
                          &mqtt_task_handle, 1);
}
