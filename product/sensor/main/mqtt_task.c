#include "cJSON.h"
#include "syslog.h"
#include "esp_wifi.h"
#include "sysevent.h"
#include "sys_config.h"
#include "sys_status.h"
#include "event_ids.h"
#include "mqtt.h"
#include "syscfg.h"
#include "config.h"
#include "filelog.h"
#include "ota_task.h"
#include "sysfile.h"
#include "mqtt_topic.h"

#include "freertos/FreeRTOS.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

extern char *uptime(void);
extern void mqtt_publish_sensor_data(void);

#define HEAP_MONITOR_CRITICAL 1024
#define HEAP_MONITOR_WARNING 4096

static const char *TAG = "MQTT";

static mqtt_ctx_t *mqtt_ctx;
static char json_resp[1024];
static char json_data[1024];
static char json_fwup_resp[512];

char s_temperature[20] = { 0 };
char s_humidity[20] = { 0 };
char s_battery[20] = { 0 };
char s_co2[20] = { 0 };

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

static double round_(float var) {
  // 37.66666 * 100 =3766.66
  // 3766.66 + .5 =3767.16    for rounding off value
  // then type cast to int so value is 3767
  // then divided by 100 so the value converted into 37.67
  double value = (int)(var * 100 + .5);
  return (double)(value / 100);
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
  memset(&json_resp[0], 0x00, sizeof(json_resp));
  memcpy(&json_resp[0], output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

static char *gen_sensor_resp(char *resp_type, char *value, char *bat) {
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
  cJSON *root;
  char *output;
  double val;

  char device_id[SYSCFG_S_DEVICEID] = { 0 };

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  root = gen_resp_obj(resp_type);

  val = round_(atof(value));
  cJSON_AddItemToObject(root, REQRES_K_VALUE, cJSON_CreateNumber(val));
  val = round_(atof(bat));
  if (val >= 0) {
    cJSON_AddItemToObject(root, REQRES_K_BATTERY, cJSON_CreateNumber(val));
  }

  output = cJSON_Print(root);

  memset(&json_data[0], 0x00, sizeof(json_data));
  memcpy(&json_data[0], output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_data);

  return json_data;
}

static char *gen_devinfo_resp(void) {
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
  cJSON *root, *farm_network;
  char *output;
  wifi_ap_record_t ap_info;

  esp_wifi_sta_get_ap_info(&ap_info);

  /* ip address of STA */
  esp_netif_ip_info_t ip_info;
  char ip_addr[16] = { 0 };
  char free_mem[20] = { 0 };
  char fw_version[SYSCFG_S_FWVERSION] = { 0 };
  char reconnect[SYSCFG_S_RECONNECT] = { 0 };

  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
  esp_ip4addr_ntoa(&ip_info.ip, ip_addr, sizeof(ip_addr));

  snprintf(free_mem, sizeof(free_mem), "%d", xPortGetFreeHeapSize());
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  syscfg_get(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect, sizeof(reconnect));

  root = gen_resp_obj("devinfo");
  farm_network = cJSON_CreateObject();

  cJSON_AddItemToObject(root, REQRES_K_FWVERSION, cJSON_CreateString(fw_version));

  // Generate farm_network json object
  cJSON_AddItemToObject(root, REQRES_K_FARMNETWORK, farm_network);
  cJSON_AddItemToObject(farm_network, REQRES_K_FARMSSID, cJSON_CreateString((char *)ap_info.ssid));
  cJSON_AddItemToObject(farm_network, REQRES_K_CHANNEL, cJSON_CreateNumber(ap_info.primary));

  cJSON_AddItemToObject(root, REQRES_K_IPADDRESS, cJSON_CreateString((char *)ip_addr));
  cJSON_AddItemToObject(root, REQRES_K_RSSI, cJSON_CreateNumber(ap_info.rssi));

  cJSON_AddItemToObject(root, REQRES_K_SENDINTERVAL, cJSON_CreateNumber(MQTT_SEND_INTERVAL));
  if (!is_battery_model())
    cJSON_AddItemToObject(root, REQRES_K_POWERMODE, cJSON_CreateString("plug"));
  else
    cJSON_AddItemToObject(root, REQRES_K_POWERMODE, cJSON_CreateString("battery"));

  cJSON_AddItemToObject(root, REQRES_K_UPTIME, cJSON_CreateString(uptime()));
  cJSON_AddItemToObject(root, REQRES_K_FREEMEM, cJSON_CreateString(free_mem));
  cJSON_AddItemToObject(root, REQRES_K_RECONNECT, cJSON_CreateString(reconnect));
  cJSON_AddItemToObject(root, REQRES_K_TIMESTAMP, cJSON_CreateString(log_timestamp()));

  output = cJSON_Print(root);

  memset(&json_resp[0], 0x00, sizeof(json_resp));
  memcpy(&json_resp[0], output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

static char *gen_fwupdate_resp(int result) {
  cJSON *root;
  char *output;
  char fw_version[SYSCFG_S_FWVERSION] = { 0 };

  root = gen_resp_obj("fw_update");

  if (result == 0) {
    cJSON_AddItemToObject(root, REQRES_K_STATE, cJSON_CreateString("success"));
  } else {
    cJSON_AddItemToObject(root, REQRES_K_STATE, cJSON_CreateString("failure"));
  }
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  cJSON_AddItemToObject(root, REQRES_K_FWVERSION, cJSON_CreateString(fw_version));

  // Print json payload
  output = cJSON_Print(root);
  memset(&json_fwup_resp, 0x00, sizeof(json_fwup_resp));
  memcpy(&json_fwup_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_fwup_resp);

  return json_fwup_resp;
}

static void update_sensor_data_cmd_handler(void) {
  mqtt_publish(mqtt_response, gen_default_resp("update"), 0);
  sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
  sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
  if (is_battery_model()) {
    sysevent_set(ADC_BATTERY_EVENT, s_battery);
  }
#if (SENSOR_TYPE == SCD4X)
  sysevent_set(I2C_CO2_EVENT, s_co2);
#endif
  mqtt_publish_sensor_data();
}

static void reset_device_cmd_handler(void) {
  mqtt_publish(mqtt_response, gen_default_resp("reset"), 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void factory_reset_cmd_handler(void) {
  if (syscfg_clear(0) != 0) {
    LOGE(TAG, "Failed to sys CFG_DATA clear");
  }
  mqtt_publish(mqtt_response, gen_default_resp("factory_reset"), 0);
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
  mqtt_publish(mqtt_response, gen_default_resp("wifi_change"), 0);
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
  mqtt_publish(mqtt_response, gen_default_resp("server_change"), 0);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void serach_device_cmd_handler(void) {
  set_identification(1);
  mqtt_publish(mqtt_response, gen_default_resp("search"), 0);
}

static void spiffs_format_cmd_handler(void) {
  sysfile_format();
  mqtt_publish(mqtt_response, gen_default_resp("spiffs_format"), 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static int update_fwversion_cmd_handler(cJSON *payload) {
  cJSON *url = cJSON_GetObjectItem(payload, REQRES_K_URL);

  if (!url) {
    LOGE(TAG, "Failed to get firmware download URL from mqtt data");
    return -1;
  }

  LOGI(TAG, "fw download url = %s", url->valuestring);

  int ret = start_ota_fw_task_wait(url->valuestring);
  LOGI(TAG, "start_ota_fw_task_wait : ret = %d", ret);

  return ret;
}

static int mqtt_req_cmd_handler(cJSON *mqtt_data) {
  int ret = 0;
  cJSON *req_type = cJSON_GetObjectItem(mqtt_data, REQRES_K_TYPE);

  if (!req_type) {
    LOGE(TAG, "Failed to get req type from mqtt data");
    return -1;
  }

  /* devinfo */
  if (!strcmp(req_type->valuestring, REQRES_V_DEVINFO)) {
    mqtt_publish(mqtt_response, gen_devinfo_resp(), 0);
  }
  /* fw_update */
  else if (!strcmp(req_type->valuestring, REQRES_V_FWUPDATE)) {
    set_fwupdate(1);
    ret = update_fwversion_cmd_handler(mqtt_data);
    mqtt_publish(mqtt_response, gen_fwupdate_resp(ret), 0);
    set_fwupdate(0);
  }
  /* update */
  else if (!strcmp(req_type->valuestring, REQRES_V_UPDATE)) {
    update_sensor_data_cmd_handler();
  }
  /* reset a device */
  else if (!strcmp(req_type->valuestring, REQRES_V_RESET)) {
    reset_device_cmd_handler();
  }
  /* factory reset */
  else if (!strcmp(req_type->valuestring, REQRES_V_FACTORY_RESET)) {
    factory_reset_cmd_handler();
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
    serach_device_cmd_handler();
  }
  /* spiffs (file) format */
  else if (!strcmp(req_type->valuestring, REQRES_V_SPIFFS_FORMAT)) {
    spiffs_format_cmd_handler();
  }

  return ret;
}

// REQUEST HANDLER
void mqtt_request_handler(const mqtt_event_data_t *const mqtt_event_data) {
  cJSON *mqtt_data_obj;

  uint32_t topic_size = mqtt_event_data->topic_len + 1;
  uint32_t data_size = mqtt_event_data->payload_len + 1;

  char *mqtt_topic = (char *)malloc(topic_size);
  char *mqtt_data = (char *)malloc(data_size);

  memset(mqtt_topic, 0x00, topic_size);
  memset(mqtt_data, 0x00, data_size);

  memcpy(mqtt_topic, mqtt_event_data->topic, mqtt_event_data->topic_len);
  memcpy(mqtt_data, mqtt_event_data->payload, mqtt_event_data->payload_len);

  mqtt_data_obj = cJSON_Parse(mqtt_data);
  if (!mqtt_data_obj) {
    LOGE(TAG, "Failed to parse mqtt data");
    free(mqtt_topic);
    free(mqtt_data);
    return;
  }

  // print pretty data of mqtt request
  {
    char *mqtt_data_pretty = cJSON_Print(mqtt_data_obj);
    LOGI(TAG, "\nRequest topic data from broker:\n%s\n%s", mqtt_topic, mqtt_data_pretty);
    free(mqtt_data_pretty);
  }

  free(mqtt_topic);
  free(mqtt_data);

  /* Process the request command */
  int ret = mqtt_req_cmd_handler(mqtt_data_obj);
  LOGI(TAG, "mqtt_req_cmd_handler : ret = %d", ret);

  cJSON_Delete(mqtt_data_obj);
}

static void mqtt_event_callback(void *handler_args, int32_t event_id, void *event_data) {
  mqtt_event_data_t *event = (mqtt_event_data_t *)event_data;

  LOGI(TAG, "Event id: %d\n", event_id);
  LOGI(TAG, "event msg_id = %d\n", event->msg_id);

  switch (event_id) {
    case MQTT_EVT_ERROR: printf("event MQTT_EVT_ERROR\n"); break;
    case MQTT_EVT_CONNECTED:
      LOGI(TAG, "event mqtt broker connected\n");
      // mqtt_subscribe("cmd/GLSTH-3C61053E75B8/req", 0);
      int msg_id = mqtt_subscribe(mqtt_request, 0);
      LOGI(TAG, "mqtt_subscribe msg_id = %d", msg_id);
      break;
    case MQTT_EVT_DISCONNECTED: printf("event mqtt broker disconnected\n"); break;
    case MQTT_EVT_SUBSCRIBED: printf("event mqtt subscribed\n"); break;
    case MQTT_EVT_DATA:
      LOGI(TAG, "event MQTT_EVT_DATA\n");
      LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
      LOGI(TAG, "PAYLOAD=%.*s\r\n", event->payload_len, event->payload);
      mqtt_request_handler(event);
      break;
    default: break;
  }
}

int start_mqttc(void) {
  int ret = 0;
  char farmip[SYSCFG_S_FARMIP] = { 0 };
  char device_id[SYSCFG_S_DEVICEID] = { 0 };

  syscfg_get(SYSCFG_I_FARMIP, SYSCFG_N_FARMIP, farmip, sizeof(farmip));

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

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  snprintf(mqtt_request, sizeof(mqtt_request), CMD_REQUEST_TOPIC, device_id);
  snprintf(mqtt_response, sizeof(mqtt_response), CMD_RESPONSE_TOPIC, device_id);

  mqtt_ctx = mqtt_client_init(&config);
  ret = mqtt_client_start(mqtt_ctx);
  if (ret != 0) {
    LOGE(TAG, "Failed to mqtt start");
  }

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
  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  int ret = 0;

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  snprintf(mqtt_temperature, sizeof(mqtt_temperature), TEMPERATURE_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_humidity, sizeof(mqtt_humidity), HUMIDITY_PUB_SUB_TOPIC, device_id);

#if (SENSOR_TYPE == SCD4X)
  char mqtt_co2[80] = { 0 };
  snprintf(mqtt_co2, sizeof(mqtt_co2), CO2_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, I2C_CO2_EVENT, &s_co2, sizeof(s_co2));
#endif

#if ((SENSOR_TYPE == SHT3X) || (SENSOR_TYPE == SCD4X))
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
#endif

  if (!is_battery_model())
    snprintf(s_battery, sizeof(s_battery), "-1");
  else
    sysevent_get(SYSEVENT_BASE, ADC_BATTERY_EVENT, &s_battery, sizeof(s_battery));

#if (SENSOR_TYPE == SHT3X)
  if (s_temperature[0] && s_humidity[0]) {
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_humidity, gen_sensor_resp("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  } else {
    mqtt_publish(mqtt_temperature, gen_sensor_resp("air", "", s_battery), 0);
    mqtt_publish(mqtt_humidity, gen_sensor_resp("air", "", s_battery), 0);
  }
#elif (SENSOR_TYPE == SCD4X)
  if (s_temperature[0] && s_humidity[0] && s_co2[0]) {
    ret = mqtt_publish(mqtt_co2, gen_sensor_resp("air", s_co2, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
    ret = mqtt_publish(mqtt_humidity, gen_sensor_resp("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  } else {
    mqtt_publish(mqtt_co2, gen_sensor_resp("air", "", s_battery), 0);
    mqtt_publish(mqtt_temperature, gen_sensor_resp("air", "", s_battery), 0);
    mqtt_publish(mqtt_humidity, gen_sensor_resp("air", "", s_battery), 0);
  }
#elif (SENSOR_TYPE == RK520_02)
  char s_ec[20] = { 0 };
  char s_moisture[20] = { 0 };
  char s_temperature[20] = { 0 };
  char mqtt_ec[80] = { 0 };
  snprintf(mqtt_ec, sizeof(mqtt_ec), EC_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_EC_EVENT, s_ec, sizeof(s_ec));
  sysevent_get(SYSEVENT_BASE, MB_MOISTURE_EVENT, s_moisture, sizeof(s_moisture));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  if (s_ec[0]) {
    ret = mqtt_publish(mqtt_ec, gen_sensor_resp("air", s_ec, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == SWSR7500)
  char s_pyranometer[20] = { 0 };
  char mqtt_solar[80] = { 0 };
  snprintf(mqtt_solar, sizeof(mqtt_solar), SOLAR_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_PYRANOMETER_EVENT, s_pyranometer, sizeof(s_pyranometer));
  if (s_pyranometer[0]) {
    ret = mqtt_publish(mqtt_solar, gen_sensor_resp("air", s_pyranometer, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#endif

  heap_monitor_func(8092, 4096);
}
