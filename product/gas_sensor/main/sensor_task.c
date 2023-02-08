#include <string.h>
#include <math.h>
#include "log.h"
#include "utils.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "sys_status.h"
#include "main.h"
#include "filelog.h"
#include "config.h"
#if (CONFIG_DATALOGGER_RS_ECTH)
#include "mb_master_rtu.h"
#endif
#include "winsen_ze03.h"

#define MIN_READ_CNT 2

#define SENSOR_TEST 0

typedef enum {
  RK_TH_SLAVE_ID = 0x01,        // Rika Temperature/Humidity sensor
  RK_WD_SLAVE_ID = 0x02,        // Rika Wind direction sensor
  RK_WS_SLAVE_ID = 0x03,        // Rika Wind speed sensor
  RK_WPH_SLAVE_ID = 0x04,       // Rika Water PH sensor
  RK_SEC_SLAVE_ID = 0x05,       // Rika Soil EC sensor
  RK_SPH_SLAVE_ID = 0x06,       // Rika Soil PH sensor
  RK_WEC_SLAVE_ID = 0x07,       // Rika Water EC sensor
  KD_SOLAR_SLAVE_ID = 0x1F,     // Korea Digital Solar sensor
  KD_CO2_SLAVE_ID = 0x1F,       // Korea Digitial Co2 sensor
  RENKE_SEC_SLAVE_ID_1 = 0x01,  // EC sensor ranke sensor
  RENKE_SEC_SLAVE_ID_2 = 0x02,  // EC sensor ranke sensor
  RENKE_SEC_SLAVE_ID_3 = 0x03,  // EC sensor ranke sensor
} MB_SLAVE_ADDR;

static const char* TAG = "sensor_task";

#if (CONFIG_DATALOGGER_RS_ECTH)
int num_characteristic = 0;
mb_characteristic_info_t mb_characteristic[3] = { 0 };
#endif

RTC_DATA_ATTR float memory_sensor_buf;
RTC_DATA_ATTR uint8_t m_alive_count;

int sensor_comparison(float m_sensor, float sensor, const float const_comparison);

int alive_check_task(void) {
  int ret = 0;
  LOGI(TAG, "%d", m_alive_count);
  if (m_alive_count >= 20) {
    m_alive_count = 0;
    ret = 0;
  } else {
    m_alive_count = m_alive_count + 1;
    ret = 1;
  }
  return ret;
}

int sensor_init(void) {
  int res = 0;

  winsen_ze03_init();
  winsen_ze03_set_active(1);
#if (CONFIG_DATALOGGER_RS_ECTH)
  mb_characteristic_info_t mb_soil_ec[3] = {
    { 0, "Data_1", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID_1, MB_HOLDING_REG, 0x0000, 0x0003 },
    { 1, "Data_2", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID_2, MB_HOLDING_REG, 0x0000, 0x0003 },
    { 2, "Data_3", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID_3, MB_HOLDING_REG, 0x0000, 0x0003 },
  };
  memcpy(&mb_characteristic, mb_soil_ec, sizeof(mb_soil_ec));
  num_characteristic = 1;

  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);

  if (mb_master_init(UART_PORT_NUM, BAUD_RATE) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return -1;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  mb_master_register_characteristic(&mb_characteristic[0], num_characteristic);
#endif

  return res;
}

#if (CONFIG_DATALOGGER_RS_ECTH)
float convert_bulk_to_saturation_ec(float temp, float mos, float ec) {
  float raw, eb, ep, op, saturation_ec;

  raw = (mos + 0.6956) / 0.0003879;
  eb = (0.000000002887 * raw * raw * raw) - (0.00002080 * raw * raw) + (0.05276 * raw) - 43.39;
  eb = eb * eb;
  ep = 80.3 - (0.37 * (temp - 20));
  op = (ep * ec) / (eb - 4.1);
  saturation_ec = (80 * mos * ec) / (0.5 * (eb - 4.1));

  LOGI(TAG, "[EC] raw : %lf, eb : %lf, ep : %lf", raw, eb, ep);
  LOGI(TAG, "[EC] Bulk EC : %lf", ec);
  LOGI(TAG, "[EC] Pore Water EC : %lf (ds/m)", op);
  LOGI(TAG, "[EC] Saturation Extract EC : %lf (ds/m)", saturation_ec);

  return saturation_ec;
}

int read_soil_ec_rs_ecth(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_temperature[10] = { 0 };
  char s_moisture[10] = { 0 };
  char s_ec[10] = { 0 };
  char s_saturation_ec[10] = { 0 };
  char s_bat_volt[10] = { 0 };

  float f_temp = 0.0;
  float f_mos = 0.0;
  float f_ec = 0.0;
  float f_saturation_ec = 0.0;

  uint16_t u_temp = 0;
  uint16_t u_mos = 0;
  uint16_t u_ec = 0;

  set_log_file_write_flag(1);
  LOGE(TAG, "file write flag set!");

  sysevent_get(SYSEVENT_BASE, ADC_BATTERY_EVENT, s_bat_volt, sizeof(s_bat_volt));
  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value : %d ", i);
      // set_rs485_conn_fail(1);
      res = -1;
    } else {
      // set_rs485_conn_fail(0);
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_mos, value, 2);
      memcpy(&u_temp, value + 2, 2);
      memcpy(&u_ec, value + 4, 2);
      f_temp = (float)(u_temp / 10.00);
      f_mos = (float)(u_mos / 10.0);
      f_ec = (float)(u_ec / 1000.00);
      LOGI(TAG, "ec = %.2f", f_ec);
      LOGI(TAG, "moisture = %.2f", f_mos);
      LOGI(TAG, "temperature = %.2f", f_temp);
      snprintf(s_temperature, sizeof(s_temperature), "%.2f", f_temp);
      snprintf(s_moisture, sizeof(s_moisture), "%.2f", f_mos);
      snprintf(s_ec, sizeof(s_ec), "%.2f", f_ec);

      f_saturation_ec = convert_bulk_to_saturation_ec(f_temp, (f_mos / 100), f_ec);
      snprintf(s_saturation_ec, sizeof(s_saturation_ec), "%.2f", f_saturation_ec);
      /*
            sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
            sysevent_set(MB_MOISTURE_EVENT, s_moisture);
            sysevent_set(MB_SOIL_EC_EVENT, s_saturation_ec);
            sysevent_set(MB_SOIL_BULK_EC_EVENT, s_ec);
      */

      if (i == SEN1) {
        LOGI(TAG, "SEN1 :ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
        FDATA(SEN_DATA_PATH_1, "%.2f %.2f %.2f %s", f_saturation_ec, f_mos, f_temp, s_bat_volt);
      } else if (i == SEN2) {
        LOGI(TAG, "SEN2 :ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
        FDATA(SEN_DATA_PATH_2, "%.2f %.2f %.2f %s", f_saturation_ec, f_mos, f_temp, s_bat_volt);
      } else if (i == SEN3) {
        LOGI(TAG, "SEN3 : ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
        FDATA(SEN_DATA_PATH_3, "%.2f %.2f %.2f %s", f_saturation_ec, f_mos, f_temp, s_bat_volt);
      } else {
        LOGE(TAG, "log data not saved.");
      }

      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }

  set_log_file_write_flag(0);
  LOGE(TAG, "file write flag unset!");

  return res;
}
#endif

int sensor_read(void) {
  int rc = 0;
  winsen_ze03_read_manual();
#if (CONFIG_GASSENSOR_TEMP_HUM_EC)
  rc = read_soil_ec_rs_ecth();
#elif (CONFIG_GASSENSOR_GAS)
  rc = read_water_ph();
#endif
  return rc;
}
