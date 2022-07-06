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

#define SEND_INTERVAL 15

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *TAG = "MQTT";

static mqtt_ctx_t *mqtt_ctx;
static TaskHandle_t mqtt_handle = NULL;
static char jsonBuffer[400];

char mqtt_request[50] = { 0 };
char mqtt_response[50] = { 0 };
char power_mode[10] = { 0 };

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

  syscfg_get(CFG_DATA, "farmip", farmip, sizeof(farmip));
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));

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

static double round_(float var) {
  // 37.66666 * 100 =3766.66
  // 3766.66 + .5 =3767.16    for rounding off value
  // then type cast to int so value is 3767
  // then divided by 100 so the value converted into 37.67
  double value = (int)(var * 100 + .5);
  return (double)value / 100;
}

static char *create_json_sensor(char *type, char *value, char *bat) {
  /*
  {
    "id" : "GLSTH-0EADBEEFFEE9",
    "type": "air"
    "value" : 25.00,
    (default)"unit": "C" - (Optional),
    "battery" : 97.24 - (Optional)
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  double _buf;

  char *hostname = generate_hostname();

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
  cJSON_AddItemToObject(root, "type", cJSON_CreateString(type));
  _buf = round_(atof(value));
  cJSON_AddItemToObject(root, "value", cJSON_CreateNumber(_buf));
  _buf = round_(atof(bat));
  if (_buf >= 0) {
    cJSON_AddItemToObject(root, "battery", cJSON_CreateNumber(_buf));
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

#define MQTT_API_TEST 0
static int sensor_data_mqtt_send() {
  char mqtt_temp[50] = { 0 };
  char mqtt_humi[50] = { 0 };

  char *hostname = generate_hostname();

  snprintf(mqtt_temp, sizeof(mqtt_temp), "value/%s/temperature", hostname);
  snprintf(mqtt_humi, sizeof(mqtt_humi), "value/%s/humidity", hostname);

#if defined(MQTT_API_TEST) && MQTT_API_TEST == 1
  float random = 0;
  char buf[20] = { 0 };

  random = (float)(rand() % 10000) / 100;  // 난수 생성
  snprintf(buf, sizeof(buf), "%f", random);
  if (!strncmp(power_mode, "P", 1)) {
    mqtt_publish(mqtt_temp, create_json_sensor("air", buf, "-1"), 0);
    mqtt_publish(mqtt_humi, create_json_sensor("air", buf, "-1"), 0);
  } else if (!strncmp(power_mode, "B", 1)) {
    mqtt_publish(mqtt_temp, create_json_sensor("air", buf, "100"), 0);
    mqtt_publish(mqtt_humi, create_json_sensor("air", buf, "100"), 0);
  }

#else
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };
  char s_battery[20] = { 0 };

  sysevent_get("SYSEVENT_BASE", I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get("SYSEVENT_BASE", I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));

  if (!strncmp(power_mode, "P", 1)) {
    snprintf(s_battery, sizeof(s_battery), "-1");
  } else if (!strncmp(power_mode, "B", 1)) {
    sysevent_get("SYSEVENT_BASE", ADC_BATTERY_EVENT, &s_battery, sizeof(s_battery));
  }

  if (s_temperature[0] && s_humidity[0]) {
    mqtt_publish(mqtt_temp, create_json_sensor("air", s_temperature, s_battery), 0);
    mqtt_publish(mqtt_humi, create_json_sensor("air", s_humidity, s_battery), 0);
  } else {
    mqtt_publish(mqtt_temp, create_json_sensor("air", "", s_battery), 0);
    mqtt_publish(mqtt_humi, create_json_sensor("air", "", s_battery), 0);
  }
#endif  // MQTT_API_TEST
  free(hostname);
  return 0;
}

static void mqtt_task(void *pvParameters) {
  int ret = 0;
  int interval_cnt = SEND_INTERVAL;

  ret = init_mqtt();
  if (ret != 0) {
    LOGI(TAG, "Failed to create mqtt init");
    mqtt_handle = NULL;
    vTaskDelete(NULL);
    return;
  }

  while (1) {
    if (interval_cnt >= SEND_INTERVAL) {
      interval_cnt = 0;
      sensor_data_mqtt_send();
    } else {
      interval_cnt++;
    }
    heap_monitor_func(HEAP_MONITOR_WARNING, HEAP_MONITOR_CRITICAL);
    if (!strncmp(power_mode, "P", 1)) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    } else if (!strncmp(power_mode, "B", 1)) {
      LOGI(TAG, "Exit MQTT Task!!!");
      mqtt_handle = NULL;
      vTaskDelete(NULL);
    }
  }
}

static int passing_payload(int payload_len, char *payload) {
  char content[100] = { 0 };
  snprintf(content, payload_len + 1, "%s", payload);
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
      sensor_data_mqtt_send();
    } else if (!strncmp(get->valuestring, "reset", 5)) {
      SLOGI(TAG, "Resetting...");
      mqtt_publish(mqtt_response, create_json_resp("reset"), 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "fw_update", 9)) {
      // 기능 구현 보류
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
