#include "config.h"
#include "log.h"
#if (SENSOR_TYPE == SHT3X)
#include "sht3x_params.h"
#endif
#if (SENSOR_TYPE == SCD4X)
#include "scd4x_params.h"
#endif
#if ((SENSOR_TYPE == RK520_02) || (SENSOR_TYPE == SWSR7500))
#include "mb_master_rtu.h"
#endif
#include "utils.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "monitoring.h"
#include "filelog.h"
#include "main.h"

#include <string.h>

#define MAX_READ_CNT 10
#define MIN_READ_CNT 2

typedef enum {
  RK_SEC_SENSOR = 0,  // RK520 Soil EC sensor
  RK_WEC_SENSOR,      // RK500 Water EC sensor
  RK_WPH_SENSOR,      // RK500 Water PH sensor
  KD_SOLAR_SENSOR,    // KD SWSR7500 Solar sensor
  KD_CO2_SENSOR,      // KD KCD-HP CO2 sensor
} MB_SENSOR;

typedef enum {
  RK_TH_SLAVE_ID = 0x01,     // Rika Temperature/Humidity sensor
  RK_WD_SLAVE_ID = 0x02,     // Rika Wind direction sensor
  RK_WS_SLAVE_ID = 0x03,     // Rika Wind speed sensor
  RK_WPH_SLAVE_ID = 0x04,    // Rika Water PH sensor
  RK_SEC_SLAVE_ID = 0x05,    // Rika Soil EC sensor
  RK_SPH_SLAVE_ID = 0x06,    // Rika Soil PH sensor
  RK_WEC_SLAVE_ID = 0x07,    // Rika Water EC sensor
  KD_SOLAR_SLAVE_ID = 0x1F,  // Korea Digital Solar sensor
  KD_CO2_SLAVE_ID = 0x1F,    // Korea Digitial Co2 sensor
} MB_SLAVE_ADDR;

static const char* TAG = "sensor_task";

#if (SENSOR_TYPE == SHT3X)
sht3x_dev_t dev;
#elif (SENSOR_TYPE == SCD4X)
scd4x_dev_t dev;
#elif ((SENSOR_TYPE == RK520_02) || (SENSOR_TYPE == SWSR7500))
int num_characteristic = 0;
mb_characteristic_info_t mb_characteristic[3] = { 0 };
#endif

int sensor_init(void) {
  int res = 0;

#if (SENSOR_TYPE == SHT3X)
  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor = %d", res);
    return res;
  }
#elif (SENSOR_TYPE == SCD4X)
  // Please implement the SCD4x initialize code.
  if ((res = scd4x_init(&dev, &scd4x_params[0])) != 0) {
    LOGI(TAG, "Could not initialize SCD4X sensor = %d", res);
    return res;
  }
#elif ((SENSOR_TYPE == RK520_02) || (SENSOR_TYPE == SWSR7500))
  // Refer to the modbus function code and register in EC Water RK500-13.pdf
  // << Water EC Sensor >>
  // Slave ID = 0x07, Function Code = 0x03, Start Address = 0x0000, Read Register Len = 0x000A
  // Data type = Binary Data, Data Len = 20 bytes
  // mb_characteristic_info_t mb_characteristic[1] = { { 0, "Data", "Binary", DATA_BINARY, 20, 0x07, MB_HOLDING_REG,
  // 0x0000,
  //                                                    0x000A } };
  // << Soil EC Sensor >>
  // mb_characteristic_info_t mb_characteristic[1] = { { 0, "Data", "Binary", DATA_BINARY, 2, 0x05, MB_HOLDING_REG,
  // 0x0000,
  //                                                    0x0003 } };
  // Or
  // mb_characteristic_info_t mb_characteristic[3] = {
  //  { 0, "Temperature", "C", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0000, 0x0001 },
  //  { 1, "Moisture", "%", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0001, 0x0001 },
  //  { 2, "EC", "mS", DATA_U16, 2, 0x05, MB_HOLDING_REG, 0x0002, 0x0001 },
  // };

#if (SENSOR_TYPE == RK520_02)
  /**
  mb_characteristic_info_t mb_soil_ec[3] = {
    { 0, "Temperature", "C", DATA_U16, 2, RK_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0001 },
    { 1, "Moisture", "%", DATA_U16, 2, RK_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0001, 0x0001 },
    { 2, "EC", "mS", DATA_U16, 2, RK_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0002, 0x0001 },
  };
  **/

  mb_characteristic_info_t mb_soil_ec[1] = { { 0, "Data", "Binary", DATA_BINARY, 6, RK_SEC_SLAVE_ID, MB_HOLDING_REG,
                                               0x0000, 0x0003 } };
  memcpy(&mb_characteristic, mb_soil_ec, sizeof(mb_soil_ec));
  num_characteristic = 1;
#elif (SENSOR_TYPE == SWSR7500)
  mb_characteristic_info_t mb_solar[1] = {
    { 0, "Pyranometer", "W", DATA_U16, 2, KD_SOLAR_SLAVE_ID, MB_INPUT_REG, 0x0006, 0x0001 },
  };
  memcpy(&mb_characteristic, mb_solar, sizeof(mb_solar));
  num_characteristic = 1;
#endif

  mb_set_uart_config(MB_RX_PIN, MB_TX_PIN, RTS_UNCHANGED, CTS_UNCHANGED);

  if (mb_master_init(UART_PORT_NUM, BAUD_RATE) == -1) {
    LOGE(TAG, "Failed to initialize modbus master");
    return -1;
  } else {
    LOGI(TAG, "Success to initialize modbus master");
  }

  vTaskDelay(1000 / portTICK_RATE_MS);

  mb_master_register_characteristic(&mb_characteristic[0], num_characteristic);
#endif

  return res;
}

#if (SENSOR_TYPE == SHT3X)
int read_temperature_humidity(int op_mode, char* temperature, char* humidity) {
  int res = 0;
  int err_cnt = 0, read_cnt = 0;

  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  float sht3x_temperature[MAX_READ_CNT] = { 0 };
  float sht3x_humidity[MAX_READ_CNT] = { 0 };

  float sht3x_temperature_i2c = 0;
  float sht3x_humidity_i2c = 0;

  if (op_mode == BATTERY_OP_MODE) {
    read_cnt = MAX_READ_CNT;
  } else if (op_mode == POWER_OP_MODE) {
    read_cnt = MIN_READ_CNT;
  }
  for (int i = 0; i < read_cnt;) {
    // Get the values and do something with them.
    if ((res = sht3x_read(&dev, &sht3x_temperature[i], &sht3x_humidity[i])) == SHT3X_OK) {
      LOGI(TAG, "Temperature [°C]: %.2f", sht3x_temperature[i]);
      LOGI(TAG, "Relative Humidity [%%]: %.2f", sht3x_humidity[i]);
      i++;
    } else {
      err_cnt++;
      if (err_cnt >= 100)
        return res;
    }
    // Wait until 100 msec (cycle time) are over.
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }

  qsort(sht3x_temperature, read_cnt, sizeof(float), float_compare);
  qsort(sht3x_humidity, read_cnt, sizeof(float), float_compare);

  sht3x_temperature_i2c = float_average(&sht3x_temperature[0], read_cnt);
  sht3x_humidity_i2c = float_average(&sht3x_humidity[0], read_cnt);

  LOGI(TAG, "average Temperature [°C]: %.2f", sht3x_temperature_i2c);
  LOGI(TAG, "average Relative Humidity [%%]: %.2f", sht3x_humidity_i2c);

  memset(s_temperature, 0, sizeof(s_temperature));
  snprintf(s_temperature, sizeof(s_temperature), "%.2f", sht3x_temperature_i2c);
  memset(s_humidity, 0, sizeof(s_humidity));
  snprintf(s_humidity, sizeof(s_humidity), "%.2f", sht3x_humidity_i2c);

  memcpy(temperature, s_temperature, sizeof(s_temperature));
  memcpy(humidity, s_humidity, sizeof(s_humidity));

  return 0;
}
#endif

#if (SENSOR_TYPE == SCD4X)
int read_co2_temperature_humidity(char* co2, char* temperature, char* humidity) {
  int res = 0;

  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  uint16_t scd41_co2 = 0;
  float scd41_temperature = 0;
  float scd41_humidity = 0;

  res = scd4x_get_data_ready_flag(&dev);
  if (res == -1) {
    LOGI(TAG, "scd4x is not data ready !!!");
    return res;
  }

  vTaskDelay(5000 / portTICK_PERIOD_MS);

  res = scd4x_read_measurement(&dev, &scd41_co2, &scd41_temperature, &scd41_humidity);

  if (res) {
    LOGI(TAG, "Error executing scd4x_read_measurement(): %i", res);
  } else if (scd41_co2 == 0) {
    LOGI(TAG, "Invalid sample detected, skipping.");
    res = -1;
  } else {
    LOGI(TAG, "CO2: %u ppm", scd41_co2);
    LOGI(TAG, "Temperature [°C]: %.2f", scd41_temperature);
    LOGI(TAG, "Relative Humidity [%%]: %.2f", scd41_humidity);

    memset(s_co2, 0, sizeof(s_co2));
    snprintf(s_co2, sizeof(s_co2), "%d", scd41_co2);
    memset(s_temperature, 0, sizeof(s_temperature));
    snprintf(s_temperature, sizeof(s_temperature), "%.2f", scd41_temperature);
    memset(s_humidity, 0, sizeof(s_humidity));
    snprintf(s_humidity, sizeof(s_humidity), "%.2f", scd41_humidity);

    memcpy(co2, s_co2, sizeof(s_co2));
    memcpy(temperature, s_temperature, sizeof(s_temperature));
    memcpy(humidity, s_humidity, sizeof(s_humidity));
  }

  return res;
}
#endif

#if (SENSOR_TYPE == RK520_02)
int read_soil_ec(char* temperature, char* moisture, char* ec) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_temperature[10] = { 0 };
  char s_moisture[10] = { 0 };
  char s_ec[10] = { 0 };

  float f_temp = 0.0;
  float f_mos = 0.0;
  float f_ec = 0.0;

  uint16_t u_temp = 0;
  uint16_t u_mos = 0;
  uint16_t u_ec = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_temp, value, 2);
      memcpy(&u_mos, value + 2, 2);
      memcpy(&u_ec, value + 4, 2);
      f_temp = (float)(u_temp / 10.00);
      f_mos = (float)(u_mos / 10.00);
      f_ec = (float)(u_ec / 1000.00);
      LOGI(TAG, "ec = %.2f", f_ec);
      LOGI(TAG, "moisture = %.2f", f_mos);
      LOGI(TAG, "temperature = %.2f", f_temp);
      snprintf(s_temperature, sizeof(s_temperature), "%.2f", f_temp);
      snprintf(s_moisture, sizeof(s_moisture), "%.2f", f_mos);
      snprintf(s_ec, sizeof(s_ec), "%.2f", f_ec);
      snprintf(temperature, sizeof(s_temperature), "%s", s_temperature);
      snprintf(moisture, sizeof(s_moisture), "%s", s_moisture);
      snprintf(ec, sizeof(s_ec), "%s", s_ec);
      vTaskDelay(500 / portTICK_RATE_MS);
    }
  }
  return res;
}
#endif

#if (SENSOR_TYPE == SWSR7500)
int read_solar_radiation(char* pyranometer) {
  int res = 0;
  int data_len = 0;

  uint8_t value[30] = { 0 };
  char s_pyranometer[20] = { 0 };

  float f_pyranometer = 0.0;
  uint16_t u_pyranometer = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_pyranometer, value, 2);
      f_pyranometer = (float)(u_pyranometer / 10.00);
      LOGI(TAG, "pyranometer = %.2f", f_pyranometer);
      snprintf(s_pyranometer, sizeof(s_pyranometer), "%.2f", f_pyranometer);
      snprintf(pyranometer, sizeof(s_pyranometer), "%s", s_pyranometer);
    }
  }
  return res;
}
#endif

int sensor_comparison(float m_sensor, float sensor, const float const_comparison) {
  float comparison;

  comparison = sensor - m_sensor;

  LOGI(TAG, "sensor : %.2f, memory_sensor : %.2f, comparison: %.2f", sensor, m_sensor, comparison);

  if (comparison < 0) {
    comparison = comparison * -1;
  }

  if (comparison >= const_comparison) {
    return 1;
  }
  return 0;
}

float convert_uint32_to_float(uint16_t low, uint16_t high) {
  float fValue = 0.0;
  uint32_t value = ((uint32_t)low << 16) | high;
  memcpy(&fValue, &value, sizeof(value));
  return fValue;
}

