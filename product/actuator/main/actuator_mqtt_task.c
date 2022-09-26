#include "cJSON.h"
#include "syslog.h"
#include "esp_wifi.h"
#include "sysevent.h"
#include "sys_status.h"
#include "event_ids.h"
#include "mqtt.h"
#include "syscfg.h"
#include "sys_config.h"
#include "config.h"
#include "filelog.h"

#include "freertos/FreeRTOS.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

extern char *uptime(void);
extern char *generate_hostname(void);
extern void mqtt_publish_actuator_data(void);

static int process_payload(int payload_len, char *payload);

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *TAG = "MQTT";

static mqtt_ctx_t *mqtt_ctx;
static char json_data[400];
static char json_resp[400];
static char model_name[10];

char mqtt_request[80] = { 0 };
char mqtt_response[80] = { 0 };

uint8_t actuator_value[9] = { 0 };

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
      process_payload(event->payload_len, event->payload);
      break;
    default: break;
  }
}

#if (ACTUATOR_TYPE == SWITCH)
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
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
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

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("switch"));
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
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
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
#elif (ACTUATOR_TYPE == MOTOR)
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

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}

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
  // double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("motor"));
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

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
}

static void actuator_motor_passing(char content[300]) {
  uint8_t actuator_change[9] = { 0 };
  /*
  {
    "type": "motor",
    "auto_mode" : "on",
    "value" : [{"io" : 2, "mode" : "open"}]
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
              if (!strncmp(mode->valuestring, "open", 4)) {
                if (actuator_value[io->valueint] != 1) {
                  sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                  actuator_value[io->valueint] = 1;
                }
                actuator_change[io->valueint] = 1;
              } else if (!strncmp(mode->valuestring, "close", 5)) {
                if (actuator_value[io->valueint] != 2) {
                  sysevent_set(io->valueint + ACTUATOR_BASE, (char *)mode->valuestring);
                  actuator_value[io->valueint] = 2;
                }
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
#endif

static char *create_json_info(void) {
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
  cJSON_AddItemToObject(root, "send_interval", cJSON_CreateNumber(MQTT_SEND_INTERVAL));
  if (!is_battery_model())
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("plug"));
  else
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("battery"));

  cJSON_AddItemToObject(root, "uptime", cJSON_CreateString(uptime()));
  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));
  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_resp[0], 0, sizeof(json_resp));
  memcpy(&json_resp[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_resp);

  return json_resp;
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
  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));
  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_resp[0], 0, sizeof(json_resp));
  memcpy(&json_resp[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_resp);

  return json_resp;
}

static int process_payload(int payload_len, char *payload) {
  char content[300] = { 0 };
  snprintf(content, payload_len + 1, "%s", payload);
  cJSON *root = cJSON_Parse(content);
  cJSON *get = cJSON_GetObjectItem(root, "type");
  if (cJSON_IsString(get)) {
    if (!strncmp(get->valuestring, "devinfo", 7)) {
      mqtt_publish(mqtt_response, create_json_info(), 0);
    } else if (!strncmp(get->valuestring, "update", 6)) {
      mqtt_publish(mqtt_response, create_json_resp("update"), 0);
      mqtt_publish_actuator_data();
    } else if (!strncmp(get->valuestring, "reset", 5)) {
      mqtt_publish(mqtt_response, create_json_resp("reset"), 0);
      SLOGI(TAG, "Resetting...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "factory_reset", 13)) {
      if (syscfg_clear(0) != 0) {
        LOGE(TAG, "Failed to sys CFG_DATA clear");
      }
      mqtt_publish(mqtt_response, create_json_resp("factory_reset"), 0);
      SLOGI(TAG, "Resetting...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "server_change", 13)) {
      char farmip[30] = { 0 };
      cJSON *Server = cJSON_GetObjectItem(root, "Server");
      if (cJSON_IsString(Server)) {
        snprintf(farmip, sizeof(farmip), "%s", Server->valuestring);
        syscfg_set(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip);
        LOGI(TAG, "Got server url = %s ", Server->valuestring);
      }
      SLOGI(TAG, "Resetting...");
      mqtt_publish(mqtt_response, create_json_resp("server_change"), 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "fw_update", 9)) {
      // 기능 구현 보류
    } else if (!strncmp(get->valuestring, "switch", 6)) {
#if (ACTUATOR_TYPE == SWITCH)
      actuator_switch_passing(content);
      mqtt_publish_actuator_data();
#endif
    } else if (!strncmp(get->valuestring, "motor", 5)) {
#if (ACTUATOR_TYPE == MOTOR)
      actuator_motor_passing(content);
      mqtt_publish_actuator_data();
#endif
    }
  }
  cJSON_Delete(root);
  return 0;
}

int start_mqttc(void) {
  int ret = 0;
  char farmip[30] = { 0 };

  syscfg_get(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip, sizeof(farmip));
  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));

  if (!farmip[0]) {
    return -1;
  }

  char *s_host = strtok(farmip, ":");  //공백을 기준으로 문자열 자르기
  char *s_port = strtok(NULL, " ");    //널문자를 기준으로 다시 자르기

  if (s_host == NULL || s_port == NULL) {
    return -1;
  }

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

void stop_mqttc(void) {
  if (mqtt_ctx) {
    mqtt_client_disconnect(mqtt_ctx);
    mqtt_client_stop(mqtt_ctx);
    mqtt_client_deinit(mqtt_ctx);
    mqtt_ctx = NULL;
  }
}

void mqtt_publish_actuator_data(void) {
  char *hostname = generate_hostname();

#if (ACTUATOR_TYPE == SWITCH)
  char mqtt_switch[50] = { 0 };
  snprintf(mqtt_switch, sizeof(mqtt_switch), "value/%s/switch", hostname);
  mqtt_publish(mqtt_switch, create_json_actuator_switch(), 0);
#elif (ACTUATOR_TYPE == MOTOR)
  char mqtt_motor[50] = { 0 };
  snprintf(mqtt_motor, sizeof(mqtt_motor), "value/%s/motor", hostname);
  mqtt_publish(mqtt_motor, create_json_actuator_motor(), 0);
#endif

  free(hostname);

  heap_monitor_func(8092, 4096);
}
