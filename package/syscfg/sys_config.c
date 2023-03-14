#include "sdkconfig.h"
#include "sys_config.h"
#include "esp_mac.h"
#include "syslog.h"

#include <string.h>

#define TAG "sys_config"

#define dbg(format, args...) LOGI(TAG, format, ##args)
#define dbgw(format, args...) LOGW(TAG, format, ##args)
#define dbge(format, args...) LOGE(TAG, format, ##args)

#define SYSCFG_VALUE_SIZE 128

typedef struct syscfg_data {
  char *name;
  uint32_t index;
  uint32_t buff_len;
} syscfg_data_t;

syscfg_data_t syscfg_table[] = {
  { SYSCFG_N_SSID, SYSCFG_I_SSID, SYSCFG_S_SSID },
  { SYSCFG_N_PASSWORD, SYSCFG_I_PASSWORD, SYSCFG_S_PASSWORD },
  { SYSCFG_N_FARMIP, SYSCFG_I_FARMIP, SYSCFG_S_FARMIP },
  { SYSCFG_N_GATEWAYIP, SYSCFG_I_GATEWAYIP, SYSCFG_S_GATEWAYIP },
  { SYSCFG_N_FWVERSION, SYSCFG_I_FWVERSION, SYSCFG_S_FWVERSION },
  { SYSCFG_N_HWVERSION, SYSCFG_I_HWVERSION, SYSCFG_S_HWVERSION },
  { SYSCFG_N_MACADDRESS, SYSCFG_I_MACADDRESS, SYSCFG_S_MACADDRESS },
  { SYSCFG_N_MODELNAME, SYSCFG_I_MODELNAME, SYSCFG_S_MODELNAME },
  { SYSCFG_N_POWERMODE, SYSCFG_I_POWERMODE, SYSCFG_S_POWERMODE },
  { SYSCFG_N_SERIALNO, SYSCFG_I_SERIALNO, SYSCFG_S_SERIALNO },
  { SYSCFG_N_DEVICEID, SYSCFG_I_DEVICEID, SYSCFG_S_DEVICEID },
  { SYSCFG_N_RECONNECT, SYSCFG_I_RECONNECT, SYSCFG_S_RECONNECT },
  { SYSCFG_N_TIMEZONE, SYSCFG_I_TIMEZONE, SYSCFG_S_TIMEZONE },
  { SYSCFG_N_REGIONCODE, SYSCFG_I_REGIONCODE, SYSCFG_S_REGIONCODE },
#if (CONFIG_IRRIGATION_HID)
  { SYSCFG_N_ZONE1_FLOW, SYSCFG_I_ZONE1_FLOW, SYSCFG_S_ZONE1_FLOW },
  { SYSCFG_N_ZONE2_FLOW, SYSCFG_I_ZONE2_FLOW, SYSCFG_S_ZONE2_FLOW },
  { SYSCFG_N_ZONE3_FLOW, SYSCFG_I_ZONE3_FLOW, SYSCFG_S_ZONE3_FLOW },
  { SYSCFG_N_ZONE4_FLOW, SYSCFG_I_ZONE4_FLOW, SYSCFG_S_ZONE4_FLOW },
  { SYSCFG_N_ZONE5_FLOW, SYSCFG_I_ZONE5_FLOW, SYSCFG_S_ZONE5_FLOW },
  { SYSCFG_N_ZONE6_FLOW, SYSCFG_I_ZONE6_FLOW, SYSCFG_S_ZONE6_FLOW },
#endif
};

#define NUM_OF_SYSCFG (sizeof(syscfg_table) / sizeof(syscfg_data_t))

static void generate_default_serial(char *serial_no, size_t buff_len) {
  int supplier_id = 10;  // Greenlabs
  int year_mfr = 21;     // 2021 ~
  int week_mfr = 01;     // 01 ~ 52 week
  int seq_id = 1;        // 00001 ~ FFFFF
  char prod_code[10] = { 0 };

#if (CONFIG_SENSOR_SHT3X || CONFIG_DATALOGGER_SHT3X)
  snprintf(prod_code, sizeof(prod_code), "%s", "TH");
#elif (CONFIG_SENSOR_SCD4X || CONFIG_DATALOGGER_SCD4X)
  snprintf(prod_code, sizeof(prod_code), "%s", "CO");
#elif (CONFIG_SENSOR_RK520_02 || CONFIG_DATALOGGER_RK520_02 || CONFIG_DATALOGGER_RS_ECTH || CONFIG_SENSOR_RS_ECTH)
  snprintf(prod_code, sizeof(prod_code), "%s", "SE");
#elif (CONFIG_SENSOR_SWSR7500 || CONFIG_DATALOGGER_SWSR7500)
  snprintf(prod_code, sizeof(prod_code), "%s", "SO");
#elif (CONFIG_SENSOR_ATLAS_PH || CONFIG_SENSOR_RK500_02 || CONFIG_DATALOGGER_ATLAS_PH || CONFIG_DATALOGGER_RK500_02)
  snprintf(prod_code, sizeof(prod_code), "%s", "WP");
#elif (CONFIG_SENSOR_ATLAS_EC || CONFIG_SENSOR_RK500_13 || CONFIG_DATALOGGER_ATLAS_EC || CONFIG_DATALOGGER_RK500_13)
  snprintf(prod_code, sizeof(prod_code), "%s", "WE");
#elif (CONFIG_SENSOR_RK110_02 || CONFIG_DATALOGGER_RK110_02)
  snprintf(prod_code, sizeof(prod_code), "%s", "WD");
#elif (CONFIG_SENSOR_RK100_02 || CONFIG_DATALOGGER_RK100_02)
  snprintf(prod_code, sizeof(prod_code), "%s", "WS");
#endif
#if (CONFIG_ACTUATOR_SWITCH)
  snprintf(prod_code, sizeof(prod_code), "%s", "SW");
#elif (CONFIG_ACTUATOR_MOTOR)
  snprintf(prod_code, sizeof(prod_code), "%s", "MT");
#endif
#if (CONFIG_IRRIGATION_MASTER)
  snprintf(prod_code, sizeof(prod_code), "%s", "MS");
#elif (CONFIG_IRRIGATION_HID)
  snprintf(prod_code, sizeof(prod_code), "%s", "HD");
#elif (CONFIG_IRRIGATION_CHILD)
  snprintf(prod_code, sizeof(prod_code), "%s", "CH");
#endif
#if (CONFIG_BS_PLATFORM_GASSENSOR)
  snprintf(prod_code, sizeof(prod_code), "%s", "GS");
#endif
  snprintf(serial_no, buff_len, "%02d%02d%02d%s%05d", supplier_id, year_mfr, week_mfr, prod_code, seq_id);
}

/*----------------------------------------------------------------------------*/
int get_syscfg_idx(char *name) {
  int idx;

  if (!name) {
    return -1;
  }

  for (idx = 0; idx < NUM_OF_SYSCFG; idx++) {
    syscfg_data_t *it = &syscfg_table[idx];
    if (!strcmp((char *)it->name, name)) {
      return it->index;
    }
  }

  return -1;
}
/*----------------------------------------------------------------------------*/

cJSON *dump_syscfg_to_json_object(void) {
  cJSON *root_obj = cJSON_CreateObject();

  if (root_obj) {
    int idx;
    for (idx = 0; idx < NUM_OF_SYSCFG; idx++) {
      syscfg_data_t *it;
      char *it_value;
      char syscfg_val[SYSCFG_VALUE_SIZE] = { 0 };

      it = &syscfg_table[idx];
      syscfg_get(it->index, (char *)it->name, syscfg_val, sizeof(syscfg_val));
      it_value = strlen(syscfg_val) ? syscfg_val : "";
      cJSON_AddItemToObject(root_obj, (char *)it->name, cJSON_CreateString(it_value));
    }
  }
  return root_obj;
}
/*----------------------------------------------------------------------------*/

/* NOTE: Caller owns the created syscfg object and should deallocate it */
cJSON *make_syscfg_json_object(void) {
  cJSON *root_obj, *syscfg_obj;

  root_obj = cJSON_CreateObject();
  if (!root_obj) {
    dbge("Failed to create root object");
    return NULL;
  }

  syscfg_obj = dump_syscfg_to_json_object();
  if (syscfg_obj) {
    cJSON_AddItemToObject(root_obj, "syscfg", syscfg_obj);
  } else {
    dbgw("Failed to create syscfg object");
  }

  return root_obj;
}
/*----------------------------------------------------------------------------*/

char *make_syscfg_json(void) {
  char *str = NULL;
  cJSON *syscfg_obj = make_syscfg_json_object();
  if (syscfg_obj) {
    str = cJSON_Print(syscfg_obj);
    cJSON_Delete(syscfg_obj);
  }
  return str;
}

void dump_syscfg(void) {
  char *syscfg_json = make_syscfg_json();
  if (syscfg_json) {
    dbg("\n-- Dump Syscfg --\n%s", syscfg_json);
    free(syscfg_json);
  }
}

void generate_syscfg(void) {
  uint8_t mac[6] = { 0 };
  char serial_no[SYSCFG_S_SERIALNO] = { 0 };
  char model_name[SYSCFG_S_MODELNAME] = { 0 };
  char hw_version[SYSCFG_S_HWVERSION] = { 0 };
  char device_id[SYSCFG_S_DEVICEID] = { 0 };
  char fw_version[SYSCFG_S_FWVERSION] = { 0 };
  char mac_address[SYSCFG_S_MACADDRESS] = { 0 };
  char region_code[SYSCFG_S_REGIONCODE] = { 0 };
  char power_mode[SYSCFG_S_POWERMODE] = { 0 };
  char send_interval[SYSCFG_S_SEND_INTERVAL] = { 0 };
  char op_time[SYSCFG_S_OP_TIME] = { 0 };
  char reconnect[SYSCFG_S_RECONNECT] = { 0 };

  /* Check if syscfg variable exists and set default value if not present */
  syscfg_get(SYSCFG_I_SERIALNO, SYSCFG_N_SERIALNO, serial_no, sizeof(serial_no));
  if (serial_no[0] == 0) {
    // It should be created during manufacturing process
    // Make a serial number as default value (initial value)
    generate_default_serial(serial_no, sizeof(serial_no));
    syscfg_set(SYSCFG_I_SERIALNO, SYSCFG_N_SERIALNO, serial_no);
  }
  syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  if (model_name[0] == 0) {
#if (CONFIG_SENSOR_SHT3X)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSTH");
#elif (CONFIG_SENSOR_SCD4X)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSCO");
#elif (CONFIG_SENSOR_RK520_02 || CONFIG_SENSOR_RS_ECTH)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSSE");
#elif (CONFIG_SENSOR_SWSR7500)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSSO");
#elif (CONFIG_SENSOR_ATLAS_PH || CONFIG_SENSOR_RK500_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSWP");
#elif (CONFIG_SENSOR_ATLAS_EC || CONFIG_SENSOR_RK500_13)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSWE");
#elif (CONFIG_SENSOR_RK110_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSWD");
#elif (CONFIG_SENSOR_RK100_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLSWS");
#endif
#if (CONFIG_ACTUATOR_SWITCH)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLASW");
#elif (CONFIG_ACTUATOR_MOTOR)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLAMT");
#endif
#if (CONFIG_IRRIGATION_MASTER)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLIMS");
#elif (CONFIG_IRRIGATION_HID)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLIHD");
#elif (CONFIG_IRRIGATION_CHILD)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLICH");
#endif
#if (CONFIG_DATALOGGER_SHT3X)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDTH");
#elif (CONFIG_DATALOGGER_SCD4X)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDCO");
#elif (CONFIG_DATALOGGER_RK520_02 || CONFIG_DATALOGGER_RS_ECTH)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDSE");
#elif (CONFIG_DATALOGGER_SWSR7500)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDSO");
#elif (CONFIG_DATALOGGER_ATLAS_PH || CONFIG_DATALOGGER_RK500_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDWP");
#elif (CONFIG_DATALOGGER_ATLAS_EC || CONFIG_DATALOGGER_RK500_13)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDWE");
#elif (CONFIG_DATALOGGER_RK110_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDWD");
#elif (CONFIG_DATALOGGER_RK100_02)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLDWS");
#endif
#if (CONFIG_BS_PLATFORM_GASSENSOR)
    syscfg_set(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, "GLGAS");
#endif
    syscfg_get(SYSCFG_I_MODELNAME, SYSCFG_N_MODELNAME, model_name, sizeof(model_name));
  }
  syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  if (power_mode[0] == 0) {
#if (CONFIG_BS_PLATFORM_ACTUATORS || CONFIG_BS_PLATFORM_GASSENSOR)
    syscfg_set(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, "P");
#else
    syscfg_set(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, "B");
#endif
    syscfg_get(SYSCFG_I_POWERMODE, SYSCFG_N_POWERMODE, power_mode, sizeof(power_mode));
  }
  syscfg_get(SYSCFG_I_HWVERSION, SYSCFG_N_HWVERSION, hw_version, sizeof(hw_version));
  if (hw_version[0] == 0) {
    syscfg_set(SYSCFG_I_HWVERSION, SYSCFG_N_HWVERSION, "20");
  }
  syscfg_get(SYSCFG_I_MACADDRESS, SYSCFG_N_MACADDRESS, mac_address, sizeof(mac_address));
  if (mac_address[0] == 0) {
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(mac_address, sizeof(mac_address), "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
             mac[5]);
    syscfg_set(SYSCFG_I_MACADDRESS, SYSCFG_N_MACADDRESS, mac_address);
  }
  syscfg_get(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id, sizeof(device_id));
  if (device_id[0] == 0) {
    if (model_name[0] && mac_address[0]) {
      snprintf(device_id, sizeof(device_id), "%s-%s", model_name, mac_address);
      syscfg_set(SYSCFG_I_DEVICEID, SYSCFG_N_DEVICEID, device_id);
    }
  }
  syscfg_get(SYSCFG_I_REGIONCODE, SYSCFG_N_REGIONCODE, region_code, sizeof(region_code));
  if (region_code[0] == 0) {
    snprintf(region_code, sizeof(region_code), "%s", "KR");
    syscfg_set(SYSCFG_I_REGIONCODE, SYSCFG_N_REGIONCODE, region_code);
  }
  syscfg_get(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, fw_version, sizeof(fw_version));
  if (fw_version[0] == 0) {
    syscfg_set(SYSCFG_I_FWVERSION, SYSCFG_N_FWVERSION, FW_VERSION);
  }
  syscfg_get(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, send_interval, sizeof(send_interval));
  if (send_interval[0] == 0) {
    if (!strncmp(power_mode, "B", 1)) {
#if (CONFIG_BS_PLATFORM_DATALOGGER)
      syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, "600");
#else
      syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, "60");
#endif
    } else {
#if (CONFIG_BS_PLATFORM_GASSENSOR)
      syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, "300");
#else
      syscfg_set(SYSCFG_I_SEND_INTERVAL, SYSCFG_N_SEND_INTERVAL, "30");
#endif
    }
  }

  syscfg_get(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, op_time, sizeof(op_time));
  if (op_time[0] == 0) {
    if (!strncmp(power_mode, "B", 1)) {
      syscfg_set(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, "0620");
    } else {
      syscfg_set(SYSCFG_I_OP_TIME, SYSCFG_N_OP_TIME, "0620");
    }
  }

  syscfg_get(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, reconnect, sizeof(reconnect));
  if (reconnect[0] == 0) {
    syscfg_set(SYSCFG_I_RECONNECT, SYSCFG_N_RECONNECT, "0");
  }
}
