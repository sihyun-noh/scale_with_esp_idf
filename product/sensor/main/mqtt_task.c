#include "cJSON.h"
#include "syslog.h"
#include "esp_wifi.h"
#include "sysevent.h"
#include "sys_status.h"
#include "event_ids.h"
#include "mqtt.h"
#include "syscfg.h"
#include "config.h"
#include "filelog.h"
#include "ota_task.h"

#include "freertos/FreeRTOS.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

extern char *uptime(void);
extern char *generate_hostname(void);
extern void mqtt_publish_sensor_data(void);

static int process_payload(int payload_len, char *payload);

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *TAG = "MQTT";

static mqtt_ctx_t *mqtt_ctx;
static char json_resp[400];
static char json_data[400];
static char json_fwup_resp[400];
static char power_mode[10];

char s_temperature[20] = { 0 };
char s_humidity[20] = { 0 };
char s_battery[20] = { 0 };
char s_co2[20] = { 0 };

char mqtt_request[80] = { 0 };
char mqtt_response[80] = { 0 };

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

static double round_(float var) {
  // 37.66666 * 100 =3766.66
  // 3766.66 + .5 =3767.16    for rounding off value
  // then type cast to int so value is 3767
  // then divided by 100 so the value converted into 37.67
  double value = (int)(var * 100 + .5);
  return (double)(value / 100);
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
  memset(&json_data[0], 0, sizeof(json_data));
  memcpy(&json_data[0], p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_data);

  return json_data;
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
  char free_mem[20] = { 0 };
  char fw_version[80] = { 0 };
  char sleep_mode_cnt[20] = { 0 };

  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
  esp_ip4addr_ntoa(&ip_info.ip, ip_addr, sizeof(ip_addr));

  snprintf(free_mem, sizeof(free_mem), "%d", xPortGetFreeHeapSize());
  syscfg_get(CFG_DATA, "fw_version", fw_version, sizeof(fw_version));
  syscfg_get(CFG_DATA, "sleep_mode_cnt", sleep_mode_cnt, sizeof(sleep_mode_cnt));

  /* create root node and array */
  root = cJSON_CreateObject();
  farm_network = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("devinfo"));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
  cJSON_AddItemToObject(root, "fw_version", cJSON_CreateString(fw_version));

  cJSON_AddItemToObject(root, "farm_network", farm_network);

  cJSON_AddItemToObject(farm_network, "ssid", cJSON_CreateString((char *)ap_info.ssid));
  cJSON_AddItemToObject(farm_network, "channel", cJSON_CreateNumber(ap_info.primary));
  cJSON_AddItemToObject(root, "ip_address", cJSON_CreateString((char *)ip_addr));
  cJSON_AddItemToObject(root, "rssi", cJSON_CreateNumber(ap_info.rssi));
  cJSON_AddItemToObject(root, "send_interval", cJSON_CreateNumber(MQTT_SEND_INTERVAL));
  if (!strncmp(power, "P", 1)) {
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("plug"));
  } else if (!strncmp(power, "B", 1)) {
    cJSON_AddItemToObject(root, "power", cJSON_CreateString("battery"));
  }
  cJSON_AddItemToObject(root, "uptime", cJSON_CreateString(uptime()));
  cJSON_AddItemToObject(root, "free_mem", cJSON_CreateString(free_mem));
  cJSON_AddItemToObject(root, "sleep_mode", cJSON_CreateString(sleep_mode_cnt));
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

static char *create_json_fwup_resp(int ret) {
  /*
  {
    "type": "fw_update"
    "id" : "GLSTH-0EADBEEFFEE9",
    "state": "success"
    "fw_version" : "1.0.1"
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  char *p_out;
  cJSON *root;
  char *hostname = generate_hostname();
  char fw_version[80] = { 0 };

  /* create root node and array */
  root = cJSON_CreateObject();

  cJSON_AddItemToObject(root, "type", cJSON_CreateString("fw_update"));
  cJSON_AddItemToObject(root, "id", cJSON_CreateString(hostname));
  if (ret == 0) {
    cJSON_AddItemToObject(root, "state", cJSON_CreateString("success"));
  } else {
    cJSON_AddItemToObject(root, "state", cJSON_CreateString("failure"));
  }
  syscfg_get(CFG_DATA, "fw_version", fw_version, sizeof(fw_version));
  cJSON_AddItemToObject(root, "fw_version", cJSON_CreateString(fw_version));
  cJSON_AddItemToObject(root, "timestamp", cJSON_CreateString(log_timestamp()));
  /* print everything */
  p_out = cJSON_Print(root);
  memset(&json_fwup_resp, 0, sizeof(json_fwup_resp));
  memcpy(&json_fwup_resp, p_out, strlen(p_out));

  free(hostname);
  free(p_out);
  cJSON_Delete(root);

  printf("%s\r\n", json_fwup_resp);

  return json_fwup_resp;
}

static int process_payload(int payload_len, char *payload) {
  char content[100] = { 0 };
  snprintf(content, payload_len + 1, "%s", payload);
  cJSON *root = cJSON_Parse(content);
  cJSON *get = cJSON_GetObjectItem(root, "type");
  if (cJSON_IsString(get)) {
    LOGI(TAG, "get->valuestring = %s\n", get->valuestring);
    if (!strncmp(get->valuestring, "devinfo", 7)) {
      LOGI(TAG, "Got devinfo mqtt topic");
      mqtt_publish(mqtt_response, create_json_info(power_mode), 0);
    } else if (!strncmp(get->valuestring, "fw_update", strlen("fw_update"))) {
      LOGI(TAG, "Got fw_update mqtt topic");
      cJSON *url = cJSON_GetObjectItem(root, "url");
      if (url) {
        // start_ota_fw_task(url->valuestring);
        LOGI(TAG, "url = %s", url->valuestring);
        set_fwupdate(1);
        int ret = start_ota_fw_task_wait(url->valuestring);
        LOGI(TAG, "start_ota_fw_task_wait : ret = %d", ret);
        mqtt_publish(mqtt_response, create_json_fwup_resp(ret), 0);
        set_fwupdate(0);
      }
    } else if (!strncmp(get->valuestring, "update", 6)) {
      mqtt_publish(mqtt_response, create_json_resp("update"), 0);
      sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
      sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
      if (!strncmp(power_mode, "B", 1)) {
        sysevent_set(ADC_BATTERY_EVENT, s_battery);
      }
#if (SENSOR_TYPE == SCD4X)
      sysevent_set(I2C_CO2_EVENT, s_co2);
#endif
      mqtt_publish_sensor_data();
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
    } else if (!strncmp(get->valuestring, "wifi_change", 11)) {
      char farmssid[50] = { 0 };
      char farmpw[50] = { 0 };
      cJSON *ssid = cJSON_GetObjectItem(root, "SSID");
      cJSON *pw = cJSON_GetObjectItem(root, "Password");
      // ssid pw
      if (cJSON_IsString(ssid) && cJSON_IsString(pw)) {
        snprintf(farmssid, sizeof(farmssid), "%s", ssid->valuestring);
        snprintf(farmpw, sizeof(farmpw), "%s", pw->valuestring);
        syscfg_set(CFG_DATA, "ssid", farmssid);
        syscfg_set(CFG_DATA, "password", farmpw);
        LOGI(TAG, "Got SSID = [%s] password = [%s]", farmssid, farmpw);
      }
      SLOGI(TAG, "Resetting...");
      mqtt_publish(mqtt_response, create_json_resp("wifi_change"), 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "server_change", 13)) {
      char farmip[30] = { 0 };
      cJSON *Server = cJSON_GetObjectItem(root, "Server");
      if (cJSON_IsString(Server)) {
        snprintf(farmip, sizeof(farmip), "%s", Server->valuestring);
        syscfg_set(CFG_DATA, "farmip", farmip);
        LOGI(TAG, "Got server url = %s ", Server->valuestring);
      }
      SLOGI(TAG, "Resetting...");
      mqtt_publish(mqtt_response, create_json_resp("server_change"), 0);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else if (!strncmp(get->valuestring, "search", 6)) {
      set_identification(1);
      mqtt_publish(mqtt_response, create_json_resp("search"), 0);
    }
  }
  cJSON_Delete(root);
  return 0;
}

int start_mqttc(void) {
  int ret = 0;
  char farmip[30] = { 0 };

  syscfg_get(CFG_DATA, "farmip", farmip, sizeof(farmip));
  syscfg_get(MFG_DATA, "power_mode", power_mode, sizeof(power_mode));

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

void mqtt_publish_sensor_data(void) {
  char mqtt_temperature[80] = { 0 };
  char mqtt_humidity[80] = { 0 };
  int ret = 0;

  char *hostname = generate_hostname();

  snprintf(mqtt_temperature, sizeof(mqtt_temperature), "value/%s/temperature", hostname);
  snprintf(mqtt_humidity, sizeof(mqtt_humidity), "value/%s/humidity", hostname);

#if (SENSOR_TYPE == SCD4X)
  char mqtt_co2[80] = { 0 };
  snprintf(mqtt_co2, sizeof(mqtt_co2), "value/%s/co2", hostname);
  sysevent_get(SYSEVENT_BASE, I2C_CO2_EVENT, &s_co2, sizeof(s_co2));
#endif

#if ((SENSOR_TYPE == SHT3X) || (SENSOR_TYPE == SCD4X))
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
#endif

  if (!strncmp(power_mode, "P", 1)) {
    snprintf(s_battery, sizeof(s_battery), "-1");
  } else if (!strncmp(power_mode, "B", 1)) {
    sysevent_get(SYSEVENT_BASE, ADC_BATTERY_EVENT, &s_battery, sizeof(s_battery));
  }

#if (SENSOR_TYPE == SHT3X)
  if (s_temperature[0] && s_humidity[0]) {
    ret = mqtt_publish(mqtt_temperature, create_json_sensor("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_humidity, create_json_sensor("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  } else {
    mqtt_publish(mqtt_temperature, create_json_sensor("air", "", s_battery), 0);
    mqtt_publish(mqtt_humidity, create_json_sensor("air", "", s_battery), 0);
  }
#elif (SENSOR_TYPE == SCD4X)
  if (s_temperature[0] && s_humidity[0] && s_co2[0]) {
    ret = mqtt_publish(mqtt_co2, create_json_sensor("air", s_co2, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_temperature, create_json_sensor("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_humidity, create_json_sensor("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  } else {
    mqtt_publish(mqtt_co2, create_json_sensor("air", "", s_battery), 0);
    mqtt_publish(mqtt_temperature, create_json_sensor("air", "", s_battery), 0);
    mqtt_publish(mqtt_humidity, create_json_sensor("air", "", s_battery), 0);
  }
#elif (SENSOR_TYPE == RK520_02)
  char s_ec[20] = { 0 };
  char s_moisture[20] = { 0 };
  char s_temperature[20] = { 0 };
  char mqtt_ec[80] = { 0 };
  snprintf(mqtt_ec, sizeof(mqtt_ec), "value/%s/ec", hostname);
  sysevent_get(SYSEVENT_BASE, MB_EC_EVENT, s_ec, sizeof(s_ec));
  sysevent_get(SYSEVENT_BASE, MB_MOISTURE_EVENT, s_moisture, sizeof(s_moisture));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  if (s_ec[0]) {
    ret = mqtt_publish(mqtt_ec, create_json_sensor("air", s_ec, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == SWSR7500)
  char s_pyranometer[20] = { 0 };
  char mqtt_solar[80] = { 0 };
  snprintf(mqtt_solar, sizeof(mqtt_solar), "value/%s/solar", hostname);
  sysevent_get(SYSEVENT_BASE, MB_PYRANOMETER_EVENT, s_pyranometer, sizeof(s_pyranometer));
  if (s_pyranometer[0]) {
    ret = mqtt_publish(mqtt_solar, create_json_sensor("air", s_pyranometer, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#endif

  free(hostname);

  heap_monitor_func(8092, 4096);
}
