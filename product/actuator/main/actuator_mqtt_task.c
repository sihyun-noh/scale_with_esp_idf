#include <string.h>
#include "cJSON.h"
#include "syslog.h"
#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "sysevent.h"
#include "event_ids.h"
#include "mqtt.h"
#include "syscfg.h"

#include <math.h>
#include <stdlib.h>

extern char *uptime(void);
extern char *generate_hostname(void);

static int passing_payload(int payload_len, char *payload);

#define SEND_INTERVAL 0

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *TAG = "MQTT";

static mqtt_ctx_t *mqtt_ctx;
static TaskHandle_t mqtt_handle = NULL;
static char jsonBuffer[500];

char mqtt_request[50] = { 0 };
char mqtt_response[50] = { 0 };
char power_mode[10] = { 0 };
char model_name[10] = { 0 };

static int minHeap = 0;
static void heap_monitor_func(int warning_level, int critical_level) {
  int freeHeap = xPortGetFreeHeapSize();
  if (minHeap == 0 || freeHeap < minHeap) {
    minHeap = freeHeap;
  }

  LOGI(TAG, "Free HeapSize = %d", freeHeap);

  if (minHeap <= critical_level) {
    SLOGE(TAG, "Heap critical level reached: %d", critical_level);
  } else if (minHeap <= warning_level) {
    SLOGW(TAG, "Heap warning level reached: %d", warning_level);
  }
}

static int mqtt_publish(char *topic, char *payload, int qos) {
  int len = 0;
  int retain = 0;

  int msg_id = mqtt_client_publish(mqtt_ctx, topic, payload, len, qos, retain);
  return msg_id;
}

static int mqtt_subscribe(char *topic, int qos) {
  int msg_id = mqtt_client_subscribe(mqtt_ctx, topic, qos);
  return msg_id;
}

static void mqtt_event_callback(void *handler_args, int32_t event_id, void *event_data) {
  mqtt_event_data_t *event = (mqtt_event_data_t *)event_data;

  printf("Event id: %d\n", event_id);
  printf("event msg_id = %d\n", event->msg_id);
  switch (event_id) {
    case MQTT_EVT_ERROR: printf("event MQTT_EVT_ERROR\n"); break;
    case MQTT_EVT_CONNECTED:
      printf("event mqtt broker connected\n");
      // mqtt_subscribe("cmd/GLSTH-3C61053E75B8/req", 0);
      int msg_id = mqtt_subscribe(mqtt_request, 0);
      LOGI(TAG, "mqtt_subscribe msg_id = %d", msg_id);
      break;
    case MQTT_EVT_DISCONNECTED: printf("event mqtt broker disconnected\n"); break;
    case MQTT_EVT_SUBSCRIBED: printf("event mqtt subscribed\n"); break;
    case MQTT_EVT_DATA:
      printf("event MQTT_EVT_DATA\n");
      printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      printf("PAYLOAD=%.*s\r\n", event->payload_len, event->payload);
      passing_payload(event->payload_len, event->payload);
      break;
    default: break;
  }
}

static int init_mqtt() {
  int ret = 0;
  char farmip[30] = { 0 };

  syscfg_get(MFG_DATA, "model_name", model_name, sizeof(model_name));
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));
  syscfg_get(CFG_DATA, "farmip", farmip, sizeof(farmip));

  char *s_host = strtok(farmip, ":");  //공백을 기준으로 문자열 자르기
  char *s_port = strtok(NULL, " ");    //널문자를 기준으로 다시 자르기
  int n_port = atoi(s_port);

  mqtt_config_t config = {
    .transport = MQTT_TCP,
    .event_cb_fn = mqtt_event_callback,
    // .uri = "mqtt://mqtt.eclipseprojects.io",
    .host = s_host,
    .port = n_port,
  };

  char *hostname = generate_hostname();

  snprintf(mqtt_request, sizeof(mqtt_request), "cmd/%s/req", hostname);
  snprintf(mqtt_response, sizeof(mqtt_response), "cmd/%s/resp", hostname);

  mqtt_ctx = mqtt_client_init(&config);

  ret = mqtt_client_start(mqtt_ctx);
  if (ret != 0) {
    LOGE(TAG, "Failed to mqtt start");
  }
  vTaskDelay(3000 / portTICK_PERIOD_MS);

  free(hostname);
  return ret;
}

uint8_t actuator_value[9] = { 0 };
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
  // double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
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
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

static char *create_json_actuator_motor(void) {
  /*
  {
    "id" : "GLAMT-0EADBEEFFEE9",
    "auto_mode" : "on",
    "value" : [ {"io" : 1, "mode" : "fwd"},
                {"io" : 2, "mode" : "rwd"},
                {"io" : 3, "mode" : "rwd"},
                {"io" : 4, "mode" : "stop"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  // double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
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
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("fwd"));
    } else if (actuator_value[i + 1] == 2) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("rwd"));
    } else if (actuator_value[i + 1] == 0) {
      cJSON_AddItemToObject(io, "mode", cJSON_CreateString("stop"));
    }
    cJSON_AddItemToArray(value, io);
  }

  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));

  /* print everything */
  p_out = cJSON_Print(root);
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

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
  // double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("update"));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
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
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

static char *create_json_actuator_motor_resp(uint8_t actuator_change[9]) {
  /*
  {
    "id" : "GLAMT-0EADBEEFFEE9",
    "auto_mode" : "on",
    "value" : [{"io" : 1, "mode" : "fwd"},
                {"io" : 2, "mode" : "rwd"},
                {"io" : 3, "mode" : "rwd"},
                {"io" : 4, "mode" : "stop"}],
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  // double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("update"));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
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
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("fwd"));
      } else if (actuator_value[i + 1] == 2) {
        cJSON_AddItemToObject(io, "mode", cJSON_CreateString("rwd"));
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
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

static char *create_json_info(char *power) {
  /*
  {
    "type":"devinfo",
    "id" : "GLSTH-0EADBEEFFEE9",
    "fw_version": "1.0.0",
    "farm_network" : {"ssid": "connected_farm_network-ssid", "channel": 1},
    "ip_address" : "192.168.50.100",
    "rssi" : "30",
    "send_interval" : 180,
    "power": "battery",
    "uptime" : "",
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root, *farm_network;
  wifi_ap_record_t ap_info;
  char *hostname = generate_hostname();
  esp_wifi_sta_get_ap_info(&ap_info);

  /* IP Addr assigned to STA */
  esp_netif_ip_info_t ip_info;
  char ip_addr[16] = { 0 };

  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
  esp_ip4addr_ntoa(&ip_info.ip, ip_addr, sizeof(ip_addr));

  /* create root node and array */
  root = cJSON_CreateObject();
  farm_network = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("devinfo"));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
  cJSON_AddItemToObject(root, "fw_version", cJSON_CreateString(FW_VERSION));

  cJSON_AddItemToObject(root, "farm_network", farm_network);

  cJSON_AddItemToObject(farm_network, "ssid", cJSON_CreateString((char *)ap_info.ssid));
  cJSON_AddItemToObject(farm_network, "channel", cJSON_CreateNumber(ap_info.primary));
  cJSON_AddItemToObject(root, "ip_address", cJSON_CreateString((char *)ip_addr));
  cJSON_AddItemToObject(root, "rssi", cJSON_CreateNumber(ap_info.rssi));
  cJSON_AddItemToObject(root, "send_interval", cJSON_CreateNumber(SEND_INTERVAL));
  if (!strncmp(power, "P", 1)) {
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("plug"));
  } else if (!strncmp(power, "B", 1)) {
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("battery"));
  }
  cJSON_AddItemToObject(root, "uptime", cJSON_CreateString(uptime()));
  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));
  /* print everything */
  p_out = cJSON_Print(root);
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

static char *create_json_resp(char *type) {
  /*
  {
    "type": "update", "reset"
    "id" : "GLSTH-0EADBEEFFEE9",
    "state": "success"
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString(type));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
  cJSON_AddItemToObject(root, "state", cJSON_CreateString("success"));
  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));
  /* print everything */
  p_out = cJSON_Print(root);
  memset(&jsonBuffer[0], 0, sizeof(jsonBuffer));
  memcpy(&jsonBuffer[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", jsonBuffer);

  return jsonBuffer;
}

static int actuator_data_mqtt_send() {
  char *hostname = generate_hostname();

  if (!strncmp(model_name, "GLASW", 5)) {
    char mqtt_switch[50] = { 0 };
    snprintf(mqtt_switch, sizeof(mqtt_switch), "value/%s/switch", hostname);
    mqtt_publish(mqtt_switch, create_json_actuator_switch(), 0);
  } else if (!strncmp(model_name, "GLAMT", 5)) {
    char mqtt_motor[50] = { 0 };
    snprintf(mqtt_motor, sizeof(mqtt_motor), "value/%s/motor", hostname);
    mqtt_publish(mqtt_motor, create_json_actuator_motor(), 0);
  }

  free(hostname);
  return 0;
}

static void mqtt_task(void *pvParameters) {
  int ret = 0;

  ret = init_mqtt();
  if (ret != 0) {
    LOGI(TAG, "Failed to create mqtt init");
    mqtt_handle = NULL;
    vTaskDelete(NULL);
    return;
  }
  actuator_data_mqtt_send();

  while (1) {
    heap_monitor_func(HEAP_MONITOR_WARNING, HEAP_MONITOR_CRITICAL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

static void actuator_switch_passing(char content[300]) {
  uint8_t actuator_change[9] = { 0 };
  /*
  {
    "type": "switch",
    "auto_mode" : "on",
    "value" : [ {"io" : 1, "mode" : "on"},
                {"io" : 2, "mode" : "off"}]
  }
  */

  cJSON *root = cJSON_Parse(content);
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
                actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "off", 3)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 0;
                actuator_change[io->valueint] = 1;
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

  mqtt_publish(mqtt_response, create_json_actuator_switch_resp(actuator_change), 0);

  cJSON_Delete(root);
}

static void actuator_motor_passing(char content[300]) {
  uint8_t actuator_change[9] = { 0 };
  /*
  {
    "type": "motor",
    "auto_mode" : "on",
    "value" : [{"io" : 2, "mode" : "fwd"}]
  }
  */

  cJSON *root = cJSON_Parse(content);
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
              if (!strncmp(mode->valuestring, "fwd", 3)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 1;
                actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "rwd", 3)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 2;
                actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "stop", 4)) {
                sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                actuator_value[io->valueint] = 0;
                actuator_change[io->valueint] = 1;
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

  mqtt_publish(mqtt_response, create_json_actuator_motor_resp(actuator_change), 0);

  cJSON_Delete(root);
}

static int passing_payload(int payload_len, char *payload) {
  char content[300] = { 0 };
  snprintf(content, payload_len + 1, "%s", payload);
  LOGI(TAG, "payload_len : %d", payload_len);
  cJSON *root = cJSON_Parse(content);
  cJSON *get = cJSON_GetObjectItem(root, "type");
  if (cJSON_IsString(get)) {
    if (!strncmp(get->valuestring, "devinfo", 7)) {
      if (!strncmp(power_mode, "P", 1)) {
        mqtt_publish(mqtt_response, create_json_info(power_mode), 0);
      } else if (!strncmp(power_mode, "B", 1)) {
        mqtt_publish(mqtt_response, create_json_info(power_mode), 0);
      }
    } else if (!strncmp(get->valuestring, "update", 6)) {
      mqtt_publish(mqtt_response, create_json_resp("update"), 0);
      actuator_data_mqtt_send();
    } else if (!strncmp(get->valuestring, "reset", 5)) {
      SLOGI(TAG, "Resetting...");
      mqtt_publish(mqtt_response, create_json_resp("reset"), 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "fw_update", 9)) {
      // 기능 구현 보류
    } else if (!strncmp(get->valuestring, "switch", 6)) {
      actuator_switch_passing(content);
    } else if (!strncmp(get->valuestring, "motor", 5)) {
      actuator_motor_passing(content);
    }
  }
  cJSON_Delete(root);
  return 0;
}

void create_mqtt_task(void) {
  uint16_t stack_size = 8192;
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (mqtt_handle) {
    LOGI(TAG, "MQTT task is alreay created");
    return;
  }

  xTaskCreate((TaskFunction_t)mqtt_task, "MQTT", stack_size, NULL, task_priority, &mqtt_handle);
}