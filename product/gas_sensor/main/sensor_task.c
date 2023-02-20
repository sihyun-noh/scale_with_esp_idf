#include <string.h>
#include "log.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "sys_status.h"
#include "main.h"
#include "filelog.h"
#include "config.h"
#include "mb_master_rtu.h"
#if (CONFIG_WINSEN_ZE03_FEATURE)
#include "winsen_ze03.h"
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
#include "winsen_zce04b.h"
#endif

typedef enum {
  RENKE_SEC_SLAVE_ID = 0x01,  // RENKE Soil EC sensor
  TH01_SLAVE_ID = 0x02,       // TH01 Air temperature, humidity sensor
} MB_SLAVE_ADDR;

static const char *TAG = "sensor_task";

int num_characteristic = 2;
mb_characteristic_info_t mb_characteristic[2] = {
  { 0, "RS_ECTH", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0003 },
  { 1, "TH01", "Binary", DATA_BINARY, 6, TH01_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0003 },
};

float soil_temp = 0.0;
float soil_mos = 0.0;
float soil_ec = 0.0;

float air_temp = 0.0;
float air_mos = 0.0;

int sensor_init(void) {
  int res = 0;

#if (CONFIG_WINSEN_ZE03_FEATURE)
  winsen_ze03_init();
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
  winsen_zce04b_init();
#endif
  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);

  if (mb_master_init(UART_PORT_NUM, BAUD_RATE) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return -1;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  mb_master_register_characteristic(&mb_characteristic[0], num_characteristic);

  return res;
}

float convert_bulk_to_saturation_ec(float temp, float mos, float ec) {
  float raw, eb, saturation_ec;

  raw = (mos + 0.6956) / 0.0003879;
  eb = (0.000000002887 * raw * raw * raw) - (0.00002080 * raw * raw) + (0.05276 * raw) - 43.39;
  eb = eb * eb;
  saturation_ec = (80 * mos * ec) / (0.5 * (eb - 4.1));

  return saturation_ec;
}

int read_soil_ec_rs_ecth(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };

  float f_ec = 0.0;

  uint16_t u_temp = 0;
  uint16_t u_mos = 0;
  uint16_t u_ec = 0;

  if (mb_master_read_characteristic(mb_characteristic[0].cid, mb_characteristic[0].name, value, &data_len) == -1) {
    LOGE(TAG, "Failed to read value : %s ", mb_characteristic[0].name);
    res = -1;
  } else {
    for (int k = 0; k < data_len; k++) {
      LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
    }
    memcpy(&u_mos, value, 2);
    memcpy(&u_temp, value + 2, 2);
    memcpy(&u_ec, value + 4, 2);
    soil_temp = (float)(u_temp / 10.00);
    soil_mos = (float)(u_mos / 10.0);
    f_ec = (float)(u_ec / 1000.00);

    soil_ec = convert_bulk_to_saturation_ec(soil_temp, (soil_mos / 100), f_ec);

    LOGI(TAG, "[RS_ECTH] ec : %.2f, moisture : %.2f, temperature : %.2f", soil_ec, soil_mos, soil_temp);
  }

  return res;
}

int read_air_th(void) {
  int res = 0;
  int data_len = 0;
  uint8_t command[6] = { 0x24, 0x30, 0x31, 0x54, 0x0D, 0x0A };
  uint8_t value[40] = { 0 };

  float buf_t1 = 999;
  float buf_h = 999;

  if (mb_master_write_characteristic(mb_characteristic[1].cid, mb_characteristic[1].name, command) == -1) {
    LOGE(TAG, "Failed to write value : %s ", mb_characteristic[1].name);
    return -1;
  }

  if (mb_master_read_characteristic(mb_characteristic[1].cid, mb_characteristic[1].name, value, &data_len) == -1) {
    LOGE(TAG, "Failed to read value : %s ", mb_characteristic[1].name);
    res = -1;
  } else {
    for (int i = 0; i < data_len; i++) {
      if (value[i] == '=') {
        if (buf_t1 == 999) {
          buf_t1 = ((float)atof((char *)&value[i + 1])) * 10;
          air_temp = buf_t1 / 10.0;
        } else if (buf_h == 999) {
          buf_h = ((float)atof((char *)&value[i + 1])) * 10;
          air_mos = buf_h / 10.0;
        }
      }
      if (value[i] == 0x0a)
        break;
    }
    LOGI(TAG, "[TH01] moisture : %.1f, temperature : %.1f", air_mos, air_temp);
  }

  return res;
}

int sensor_read(void) {
  int rc = 0;

#if (CONFIG_WINSEN_ZE03_FEATURE)
  int gas_nh3 = 0;
  rc = winsen_ze03_read_manual(&gas_nh3);
  if (rc)
    return rc;
#endif
#if (CONFIG_WINSEN_ZCE04B_FEATURE)
  zce04b_data_t gas_data;
  rc = winsen_zce04b_read_manual(&gas_data);
  if (rc)
    return rc;
#endif

  rc = read_soil_ec_rs_ecth();
  if (rc)
    return rc;

  rc = read_air_th();
  if (rc)
    return rc;

  LOGI(TAG, "NH3,  CO, H2S,   O2, CH4, S_TEMP, S_MOS, S_EC, A_TEMP, A_MOS");
  LOGI(TAG, "%3d, %3d, %3d, %2.1f, %3d,  %2.2f, %2.2f, %2.2f,  %2.1f, %2.1f", gas_nh3, gas_data.co, gas_data.h2s,
       gas_data.o2, gas_data.ch4, soil_temp, soil_mos, soil_ec, air_temp, air_mos);

  set_log_file_write_flag(1);
  FDATA(DIR_PATH, "%d, %d, %d, %.1f, %d, %.2f, %.2f, %.2f, %.1f, %.1f", gas_nh3, gas_data.co, gas_data.h2s, gas_data.o2,
        gas_data.ch4, soil_temp, soil_mos, soil_ec, air_temp, air_mos);
  set_log_file_write_flag(0);

  return rc;
}
