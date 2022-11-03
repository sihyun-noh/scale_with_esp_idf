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

#if (SENSOR_TYPE == SHT3X)
char s_temperature[20] = { 0 };
char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == SCD4X)
char s_co2[20] = { 0 };
char s_temperature[20] = { 0 };
char s_humidity[20] = { 0 };
#elif (SENSOR_TYPE == RK520_02)
char s_bulk_ec[20] = { 0 };
char s_saturation_ec[20] = { 0 };
char s_moisture[20] = { 0 };
char s_temperature[20] = { 0 };
#elif (SENSOR_TYPE == RK500_02)
char s_ph[20] = { 0 };
#elif (SENSOR_TYPE == SWSR7500)
char s_pyranometer[20] = { 0 };
#elif (SENSOR_TYPE == ATLAS_PH)
char s_ph[20] = { 0 };
#elif (SENSOR_TYPE == ATLAS_EC)
char s_ec[20] = { 0 };
#elif (SENSOR_TYPE == RK110_02)
char s_wind_direction[20] = { 0 };
#elif (SENSOR_TYPE == RK100_02)
char s_wind_speed[20] = { 0 };
#elif (SENSOR_TYPE == RK500_13)
char s_ec[20] = { 0 };
char s_temperature[20] = { 0 };
#endif

char s_battery[20] = { 0 };

char mqtt_request[128] = { 0 };
char mqtt_response[128] = { 0 };

#if 0
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
#endif

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
  memset(json_resp, 0x00, sizeof(json_resp));
  memcpy(json_resp, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_resp);

  return json_resp;
}

static char *gen_sensor_resp(char *resp_type, char *value, char *bat) {
  /*
  {
    "id" : "GLSTH-0EADBEEFFEE9",
    "type": "air", or "soil", or "water"
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

  memset(json_data, 0x00, sizeof(json_data));
  memcpy(json_data, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_data);

  return json_data;
}

#if (SENSOR_TYPE == RK520_02)
static char *gen_sensor_bulkec_resp(char *resp_type, char *ec, char *temp, char *mos) {
  /*
  {
    "id" : "GLSTH-0EADBEEFFEE9",
    "type": "air", or "soil", or "water"
    "ec" : 0.04,
    "temperature" : 24.05,
    "moisture" : "0.25",
    "timestamp" : "2022-05-02 15:07:18"
  }
  */
  cJSON *root;
  char *output;
  double bulk_ec;
  double temperature;
  double moisture;

  char device_id[SYSCFG_S_DEVICEID] = { 0 };

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  root = gen_resp_obj(resp_type);

  bulk_ec = round_(atof(ec));
  cJSON_AddItemToObject(root, "ec", cJSON_CreateNumber(bulk_ec));
  temperature = round_(atof(temp));
  cJSON_AddItemToObject(root, "temperature", cJSON_CreateNumber(temperature));
  moisture = round_(atof(mos));
  cJSON_AddItemToObject(root, "moisture", cJSON_CreateNumber(moisture));

  output = cJSON_Print(root);

  memset(json_data, 0x00, sizeof(json_data));
  memcpy(json_data, output, strlen(output));

  free(output);
  cJSON_Delete(root);

  LOGI(TAG, "%s\n", json_data);

  return json_data;
}
#endif

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
  char s_send_interval[SYSCFG_S_SEND_INTERVAL] = { 0 };
  int send_interval;

  esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"), &ip_info);
  esp_ip4addr_ntoa(&ip_info.ip, ip_addr, sizeof(ip_addr));

  snprintf(free_mem, sizeof(free_mem), "%d", xPortGetFreeHeapSize());
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  syscfg_get(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect, sizeof(reconnect));
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, s_send_interval, sizeof(s_send_interval));
  send_interval = atoi(s_send_interval);

  root = gen_resp_obj("devinfo");
  farm_network = cJSON_CreateObject();

  cJSON_AddItemToObject(root, REQRES_K_FWVERSION, cJSON_CreateString(fw_version));

  // Generate farm_network json object
  cJSON_AddItemToObject(root, REQRES_K_FARMNETWORK, farm_network);
  cJSON_AddItemToObject(farm_network, REQRES_K_FARMSSID, cJSON_CreateString((char *)ap_info.ssid));
  cJSON_AddItemToObject(farm_network, REQRES_K_CHANNEL, cJSON_CreateNumber(ap_info.primary));

  cJSON_AddItemToObject(root, REQRES_K_IPADDRESS, cJSON_CreateString((char *)ip_addr));
  cJSON_AddItemToObject(root, REQRES_K_RSSI, cJSON_CreateNumber(ap_info.rssi));

  cJSON_AddItemToObject(root, REQRES_K_SENDINTERVAL, cJSON_CreateNumber(send_interval));
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

  root = gen_resp_obj("syscfg");
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
  mqtt_publish(mqtt_response, gen_devinfo_resp(), 0);
}

static void update_sensor_data_cmd_handler(cJSON *root) {
  (void)root;

  mqtt_publish(mqtt_response, gen_default_resp("update"), 0);

#if (SENSOR_TYPE == SHT3X)
  sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
  sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
#elif (SENSOR_TYPE == SCD4X)
  sysevent_set(I2C_CO2_EVENT, s_co2);
  sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
  sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
#elif (SENSOR_TYPE == RK520_02)
  sysevent_set(MB_SOIL_EC_EVENT, s_saturation_ec);
  sysevent_set(MB_SOIL_BULK_EC_EVENT, s_bulk_ec);
  sysevent_set(MB_MOISTURE_EVENT, s_moisture);
  sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
#elif (SENSOR_TYPE == RK500_02)
  sysevent_set(MB_WATER_PH_EVENT, s_ph);
#elif (SENSOR_TYPE == SWSR7500)
  sysevent_set(MB_PYRANOMETER_EVENT, s_pyranometer);
#elif (SENSOR_TYPE == ATLAS_PH)
  sysevent_set(MB_WATER_PH_EVENT, s_ph);
#elif (SENSOR_TYPE == ATLAS_EC)
  sysevent_set(I2C_EC_EVENT, s_ec);
#elif (SENSOR_TYPE == RK110_02)
  sysevent_set(MB_WIND_DIRECTION_EVENT, s_wind_direction);
#elif (SENSOR_TYPE == RK100_02)
  sysevent_set(MB_WIND_SPEED_EVENT, s_wind_speed);
#elif (SENSOR_TYPE == RK500_13)
  sysevent_set(MB_WATER_EC_EVENT, s_ec);
  sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
#endif

  if (is_battery_model()) {
    sysevent_set(ADC_BATTERY_EVENT, s_battery);
  }

  mqtt_publish_sensor_data();
}

static void reset_device_cmd_handler(cJSON *root) {
  (void)root;

  mqtt_publish(mqtt_response, gen_default_resp("reset"), 0);
  SLOGI(TAG, "Resetting...");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  esp_restart();
}

static void factory_reset_cmd_handler(cJSON *root) {
  (void)root;

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

static void serach_device_cmd_handler(cJSON *root) {
  (void)root;

  set_identification(1);
  mqtt_publish(mqtt_response, gen_default_resp("search"), 0);
}

static void spiffs_format_cmd_handler(cJSON *root) {
  (void)root;

  sysfile_format();
  mqtt_publish(mqtt_response, gen_default_resp("spiffs_format"), 0);
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

  mqtt_publish(mqtt_response, gen_fwupdate_resp(ret), 0);
  set_fwupdate(0);
}

/* "syscfg set" action */
static int syscfg_set_action(char *name, char *value) {
  int index = get_syscfg_idx(name);
  if (index < 0) {
    return -1;
  }

  if (syscfg_set(index, name, value) != 0) {
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

  if (syscfg_get(index, name, buff, buff_size) != 0) {
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
  mqtt_publish(mqtt_response, gen_syscfg_resp(resp_action, result), 0);
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
  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  int ret = 0;

  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, SYSCFG_S_DEVICEID);

  if (!is_battery_model())
    snprintf(s_battery, sizeof(s_battery), "-1");
  else
    sysevent_get(SYSEVENT_BASE, ADC_BATTERY_EVENT, &s_battery, sizeof(s_battery));

#if (SENSOR_TYPE == SHT3X)
  char mqtt_temperature[80] = { 0 };
  char mqtt_humidity[80] = { 0 };
  snprintf(mqtt_temperature, sizeof(mqtt_temperature), TEMPERATURE_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_humidity, sizeof(mqtt_humidity), HUMIDITY_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
  if (s_temperature[0]) {
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_humidity[0]) {
    ret = mqtt_publish(mqtt_humidity, gen_sensor_resp("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == SCD4X)
  char mqtt_co2[80] = { 0 };
  char mqtt_temperature[80] = { 0 };
  char mqtt_humidity[80] = { 0 };
  snprintf(mqtt_co2, sizeof(mqtt_co2), CO2_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_temperature, sizeof(mqtt_temperature), TEMPERATURE_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_humidity, sizeof(mqtt_humidity), HUMIDITY_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, I2C_CO2_EVENT, &s_co2, sizeof(s_co2));
  sysevent_get(SYSEVENT_BASE, I2C_HUMIDITY_EVENT, &s_humidity, sizeof(s_humidity));
  sysevent_get(SYSEVENT_BASE, I2C_TEMPERATURE_EVENT, &s_temperature, sizeof(s_temperature));
  if (s_co2[0]) {
    ret = mqtt_publish(mqtt_co2, gen_sensor_resp("air", s_co2, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_temperature[0]) {
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("air", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_humidity[0]) {
    ret = mqtt_publish(mqtt_humidity, gen_sensor_resp("air", s_humidity, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == RK520_02)
  char mqtt_saturation_ec[80] = { 0 };
  char mqtt_temperature[80] = { 0 };
  char mqtt_humidity[80] = { 0 };
  snprintf(mqtt_saturation_ec, sizeof(mqtt_saturation_ec), EC_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_temperature, sizeof(mqtt_temperature), TEMPERATURE_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_humidity, sizeof(mqtt_humidity), HUMIDITY_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_SOIL_EC_EVENT, s_saturation_ec, sizeof(s_saturation_ec));
  sysevent_get(SYSEVENT_BASE, MB_SOIL_BULK_EC_EVENT, s_bulk_ec, sizeof(s_bulk_ec));
  sysevent_get(SYSEVENT_BASE, MB_MOISTURE_EVENT, s_moisture, sizeof(s_moisture));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  if (s_saturation_ec[0]) {
    ret = mqtt_publish(mqtt_saturation_ec, gen_sensor_resp("soil", s_saturation_ec, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_temperature[0]) {
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("soil", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_moisture[0]) {
    ret = mqtt_publish(mqtt_humidity, gen_sensor_resp("soil", s_moisture, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_bulk_ec[0] && s_temperature[0] && s_moisture[0]) {
    char mqtt_bulk_ec[80] = { 0 };
    snprintf(mqtt_bulk_ec, sizeof(mqtt_bulk_ec), BULK_EC_PUB_SUB_TOPIC, device_id);
    ret = mqtt_publish(mqtt_bulk_ec, gen_sensor_bulkec_resp("log", s_bulk_ec, s_temperature, s_moisture), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == RK500_02)
  char mqtt_ph[80] = { 0 };
  snprintf(mqtt_ph, sizeof(mqtt_ph), PH_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_WATER_PH_EVENT, s_ph, sizeof(s_ph));
  if (s_ph[0]) {
    ret = mqtt_publish(mqtt_ph, gen_sensor_resp("water", s_ph, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == SWSR7500)
  char mqtt_pyranometer[80] = { 0 };
  snprintf(mqtt_pyranometer, sizeof(mqtt_pyranometer), SOLAR_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_PYRANOMETER_EVENT, s_pyranometer, sizeof(s_pyranometer));
  if (s_pyranometer[0]) {
    ret = mqtt_publish(mqtt_pyranometer, gen_sensor_resp("air", s_pyranometer, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == ATLAS_PH)
  char mqtt_ph[80] = { 0 };
  snprintf(mqtt_ph, sizeof(mqtt_ph), PH_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, I2C_PH_EVENT, s_ph, sizeof(s_ph));
  if (s_ph[0]) {
    ret = mqtt_publish(mqtt_ph, gen_sensor_resp("water", s_ph, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == ATLAS_EC)
  char mqtt_ec[80] = { 0 };
  snprintf(mqtt_ec, sizeof(mqtt_ec), EC_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, I2C_EC_EVENT, s_ec, sizeof(s_ec));
  if (s_ec[0]) {
    ret = mqtt_publish(mqtt_ec, gen_sensor_resp("water", s_ec, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == RK110_02)
  char mqtt_wind_direction[80] = { 0 };
  memset(s_wind_direction, 0x00, sizeof(s_wind_direction));
  snprintf(mqtt_wind_direction, sizeof(mqtt_wind_direction), WIND_DIRECTION_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_WIND_DIRECTION_EVENT, s_wind_direction, sizeof(s_wind_direction));
  LOGI(TAG, "value : %s", s_wind_direction);
  if (s_wind_direction[0]) {
    ret = mqtt_publish(mqtt_wind_direction, gen_sensor_resp("air", s_wind_direction, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#elif (SENSOR_TYPE == RK100_02)
  char mqtt_wind_speed[80] = { 0 };
  snprintf(mqtt_wind_speed, sizeof(mqtt_wind_speed), WIND_SPEED_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_WIND_SPEED_EVENT, s_wind_speed, sizeof(s_wind_speed));
  if (s_wind_speed[0]) {
    ret = mqtt_publish(mqtt_wind_speed, gen_sensor_resp("air", s_wind_speed, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }

#elif (SENSOR_TYPE == RK500_13)
  char mqtt_ec[80] = { 0 };
  char mqtt_temperature[80] = { 0 };
  snprintf(mqtt_ec, sizeof(mqtt_ec), EC_PUB_SUB_TOPIC, device_id);
  snprintf(mqtt_temperature, sizeof(mqtt_temperature), TEMPERATURE_PUB_SUB_TOPIC, device_id);
  sysevent_get(SYSEVENT_BASE, MB_WATER_EC_EVENT, s_ec, sizeof(s_ec));
  sysevent_get(SYSEVENT_BASE, MB_TEMPERATURE_EVENT, s_temperature, sizeof(s_temperature));
  LOGI(TAG, "ec: %s, temp: %s", s_ec, s_temperature);
  if (s_ec[0]) {
    ret = mqtt_publish(mqtt_ec, gen_sensor_resp("water", s_ec, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
  if (s_temperature[0]) {
    ret = mqtt_publish(mqtt_temperature, gen_sensor_resp("water", s_temperature, s_battery), 0);
    if (ret != 0) {
      FLOGI(TAG, "mqtt_publish error!");
    }
  }
#endif
  // heap_monitor_func(8092, 4096);
}
