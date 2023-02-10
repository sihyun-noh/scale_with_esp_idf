#include <string.h>
#include <math.h>

#include "log.h"
#include "utils.h"
#include "syslog.h"
#include "syscfg.h"
#include "event_ids.h"
#include "sysevent.h"
#include "sys_status.h"
#include "filelog.h"
#include "main.h"

#include "config.h"
#if (CONFIG_SENSOR_SHT3X)
#include "sht3x_params.h"
#endif
#if (CONFIG_SENSOR_SCD4X)
#include "scd4x_params.h"
#endif
#if (CONFIG_SENSOR_ATLAS_PH)
#include "atlas_ph_params.h"
#endif
#if (CONFIG_SENSOR_ATLAS_EC)
#include "atlas_ec_params.h"
#endif
#if ((CONFIG_SENSOR_RK520_02) || (CONFIG_SENSOR_SWSR7500) || (CONFIG_SENSOR_RK500_02) || (CONFIG_SENSOR_RK110_02) || \
     (CONFIG_SENSOR_RK100_02) || (CONFIG_SENSOR_RK500_13) || (CONFIG_SENSOR_RS_ECTH))
#include "mb_master_rtu.h"
#endif

#define MIN_READ_CNT 2

#define SENSOR_TEST 0

typedef enum {
  RK_TH_SLAVE_ID = 0x01,      // Rika Temperature/Humidity sensor
  RK_WD_SLAVE_ID = 0x02,      // Rika Wind direction sensor
  RK_WS_SLAVE_ID = 0x03,      // Rika Wind speed sensor
  RK_WPH_SLAVE_ID = 0x04,     // Rika Water PH sensor
  RK_SEC_SLAVE_ID = 0x05,     // Rika Soil EC sensor
  RK_SPH_SLAVE_ID = 0x06,     // Rika Soil PH sensor
  RK_WEC_SLAVE_ID = 0x07,     // Rika Water EC sensor
  KD_SOLAR_SLAVE_ID = 0x1F,   // Korea Digital Solar sensor
  KD_CO2_SLAVE_ID = 0x1F,     // Korea Digitial Co2 sensor
  RENKE_SEC_SLAVE_ID = 0x01,  // EC sensor ranke sensor
} MB_SLAVE_ADDR;

static const char* TAG = "sensor_task";

#if (CONFIG_SENSOR_SHT3X)
sht3x_dev_t dev;
#elif (CONFIG_SENSOR_SCD4X)
scd4x_dev_t dev;
#elif (CONFIG_SENSOR_ATLAS_PH)
atlas_ph_dev_t dev;
#elif (CONFIG_SENSOR_ATLAS_EC)
atlas_ec_dev_t dev;
#elif ((CONFIG_SENSOR_RK520_02) || (CONFIG_SENSOR_SWSR7500) || (CONFIG_SENSOR_RK500_02) || (CONFIG_SENSOR_RK110_02) || \
       (CONFIG_SENSOR_RK100_02) || (CONFIG_SENSOR_RK500_13) || (CONFIG_SENSOR_RS_ECTH))
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

#if (SENSOR_TEST)
  return res;
#endif

#if (CONFIG_SENSOR_SHT3X)
  if ((res = sht3x_init(&dev, &sht3x_params[0])) != SHT3X_OK) {
    LOGI(TAG, "Could not initialize SHT3x sensor = %d", res);
    return res;
  }
#elif (CONFIG_SENSOR_SCD4X)
  // Please implement the SCD4x initialize code.
  if ((res = scd4x_init(&dev, &scd4x_params[0])) != 0) {
    LOGI(TAG, "Could not initialize SCD4X sensor = %d", res);
    return res;
  }
#elif (CONFIG_SENSOR_ATLAS_PH)
  if ((res = atlas_ph_init(&dev, &atlas_ph_params[0])) != 0) {
    LOGI(TAG, "Could not initialize Atlas pH sensor = %d", res);
    return res;
  }
#elif (CONFIG_SENSOR_ATLAS_EC)
  if ((res = atlas_ec_init(&dev, &atlas_ec_params[0])) != 0) {
    LOGI(TAG, "Could not initialize Atlas EC sensor = %d", res);
    return res;
  }
#elif ((CONFIG_SENSOR_RK520_02) || (CONFIG_SENSOR_SWSR7500) || (CONFIG_SENSOR_RK500_02) || (CONFIG_SENSOR_RK110_02) || \
       (CONFIG_SENSOR_RK100_02) || (CONFIG_SENSOR_RK500_13) || (CONFIG_SENSOR_RS_ECTH))
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

#if (CONFIG_SENSOR_RK520_02)
  // Rika Soil EC Sensor
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
#elif (CONFIG_SENSOR_RS_ECTH)
  mb_characteristic_info_t mb_soil_ec[1] = {
    { 0, "Data", "Binary", DATA_BINARY, 6, RENKE_SEC_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0003 },
  };
  memcpy(&mb_characteristic, mb_soil_ec, sizeof(mb_soil_ec));
  num_characteristic = 1;
#elif (CONFIG_SENSOR_SWSR7500)
  // Korea Digital Solar Radiation(Pyranometer) Sensor
  mb_characteristic_info_t mb_solar[1] = {
    { 0, "Pyranometer", "W", DATA_U16, 2, KD_SOLAR_SLAVE_ID, MB_INPUT_REG, 0x0006, 0x0001 },
  };
  memcpy(&mb_characteristic, mb_solar, sizeof(mb_solar));
  num_characteristic = 1;
#elif (CONFIG_SENSOR_RK500_02)
  // Rika Water PH Sensor
  mb_characteristic_info_t mb_water_ph[3] = {
    { 0, "PH", "ph", DATA_U16, 2, RK_WPH_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0001 },
  };
  memcpy(&mb_characteristic, mb_water_ph, sizeof(mb_water_ph));
  num_characteristic = 1;
#elif (CONFIG_SENSOR_RK110_02)
  // Rika Wind Direction Sensor
  mb_characteristic_info_t mb_wind_direction[1] = { { 0, "Wind_direction", "degree", DATA_U16, 2, RK_WD_SLAVE_ID,
                                                      MB_HOLDING_REG, 0x0000, 0x0001 } };
  memcpy(&mb_characteristic, mb_wind_direction, sizeof(mb_wind_direction));
  num_characteristic = 1;
#elif (CONFIG_SENSOR_RK100_02)
  // Rika Wind Speed Sensor
  mb_characteristic_info_t mb_wind_speed[1] = {
    { 0, "Wind_speed", "m/s", DATA_U16, 2, RK_WS_SLAVE_ID, MB_HOLDING_REG, 0x0000, 0x0001 },
  };
  memcpy(&mb_characteristic, mb_wind_speed, sizeof(mb_wind_speed));
  num_characteristic = 1;
#elif (CONFIG_SENSOR_RK500_13)
  mb_characteristic_info_t mb_water_ec[1] = { { 0, "Data", "Binary", DATA_BINARY, 20, RK_WEC_SLAVE_ID, MB_HOLDING_REG,
                                                0x0000, 0x000A } };
  memcpy(&mb_characteristic, mb_water_ec, sizeof(mb_water_ec));
  num_characteristic = 1;
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
#endif

  return res;
}

#if (CONFIG_SENSOR_SHT3X)
int read_temperature_humidity(void) {
  int res = 0;
  int err_cnt = 0;

  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  float sht3x_temperature[MIN_READ_CNT] = { 0 };
  float sht3x_humidity[MIN_READ_CNT] = { 0 };

  float sht3x_temperature_i2c = 0;
  float sht3x_humidity_i2c = 0;

  for (int i = 0; i < MIN_READ_CNT;) {
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
    // Wait until 300 msec (cycle time) are over.
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }

  qsort(sht3x_temperature, MIN_READ_CNT, sizeof(float), float_compare);
  qsort(sht3x_humidity, MIN_READ_CNT, sizeof(float), float_compare);

  sht3x_temperature_i2c = float_average(&sht3x_temperature[0], MIN_READ_CNT);
  sht3x_humidity_i2c = float_average(&sht3x_humidity[0], MIN_READ_CNT);

  LOGI(TAG, "average Temperature [°C]: %.2f", sht3x_temperature_i2c);
  LOGI(TAG, "average Relative Humidity [%%]: %.2f", sht3x_humidity_i2c);

  memset(s_temperature, 0, sizeof(s_temperature));
  snprintf(s_temperature, sizeof(s_temperature), "%.2f", sht3x_temperature_i2c);
  memset(s_humidity, 0, sizeof(s_humidity));
  snprintf(s_humidity, sizeof(s_humidity), "%.2f", sht3x_humidity_i2c);

  if (is_battery_model()) {
    if (!alive_check_task() || sensor_comparison(memory_sensor_buf, sht3x_temperature_i2c, 0.1)) {
      LOGI(TAG, "temperature : %.2f, m_temperature : %.2f", sht3x_temperature_i2c, memory_sensor_buf);
      memory_sensor_buf = sht3x_temperature_i2c;
    } else {
      return SENSOR_NOT_PUB;
    }
  }
  sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
  sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);

  return 0;
}
#endif

#if (CONFIG_SENSOR_SCD4X)
int read_co2_temperature_humidity(void) {
  int res = 0;

  char s_co2[20] = { 0 };
  char s_temperature[20] = { 0 };
  char s_humidity[20] = { 0 };

  uint16_t scd41_co2 = 0;
  float scd41_temperature = 0;
  float scd41_humidity = 0;
  float f_co2 = 0;

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

    if (is_battery_model()) {
      f_co2 = atof(s_co2);
      if (!alive_check_task() || sensor_comparison(memory_sensor_buf, f_co2, 20)) {
        LOGI(TAG, "co2 : %.2f, m_co2 : %.2f", f_co2, memory_sensor_buf);
        memory_sensor_buf = f_co2;
      } else {
        return SENSOR_NOT_PUB;
      }
    }
    sysevent_set(I2C_CO2_EVENT, s_co2);
    sysevent_set(I2C_TEMPERATURE_EVENT, s_temperature);
    sysevent_set(I2C_HUMIDITY_EVENT, s_humidity);
  }

  return res;
}
#endif

#if (CONFIG_SENSOR_RK520_02 || CONFIG_SENSOR_RS_ECTH)
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

int read_soil_ec(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_temperature[10] = { 0 };
  char s_moisture[10] = { 0 };
  char s_ec[10] = { 0 };
  char s_saturation_ec[10] = { 0 };

  float f_temp = 0.0;
  float f_mos = 0.0;
  float f_ec = 0.0;
  float f_saturation_ec = 0.0;

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
#if (CONFIG_SENSOR_RK520_02)
      memcpy(&u_temp, value, 2);
      memcpy(&u_mos, value + 2, 2);
      memcpy(&u_ec, value + 4, 2);
#elif (CONFIG_SENSOR_RS_ECTH)
      memcpy(&u_mos, value, 2);
      memcpy(&u_temp, value + 2, 2);
      memcpy(&u_ec, value + 4, 2);
#endif
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

      sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);
      sysevent_set(MB_MOISTURE_EVENT, s_moisture);
      sysevent_set(MB_SOIL_EC_EVENT, s_saturation_ec);
      sysevent_set(MB_SOIL_BULK_EC_EVENT, s_ec);

      if (is_battery_model()) {
        LOGI(TAG, "ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
        // FLOGI(TAG, "ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
      } else {
        LOGI(TAG, "Power mode > ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
        // FLOGI(TAG, "Power mode > ec : %.2f, moisture : %.2f, temperature : %.2f", f_saturation_ec, f_mos, f_temp);
      }

      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
  return res;
}
#endif

#if (CONFIG_SENSOR_RK500_02)
int read_water_ph(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_ph[10] = { 0 };
  float f_ph = 0.0;
  uint16_t u_ph = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_ph, value, 2);
      f_ph = (float)(u_ph / 100.00);
      LOGI(TAG, "ph = %.2f", f_ph);
      snprintf(s_ph, sizeof(s_ph), "%.2f", f_ph);
      sysevent_set(MB_WATER_PH_EVENT, s_ph);

      if (is_battery_model()) {
        LOGI(TAG, "ph : %.2f", f_ph);
        // FLOGI(TAG, "ph : %.2f", f_ph);
      } else {
        LOGI(TAG, "Power mode > ph : %.2f", f_ph);
        // FLOGI(TAG, "Power mode > ph : %.2f", f_ph);
      }

      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
  return res;
}
#endif

#if (CONFIG_SENSOR_SWSR7500)
int read_solar_radiation(void) {
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
      // FLOGI(TAG, "pyranometer = %.2f", f_pyranometer);
      snprintf(s_pyranometer, sizeof(s_pyranometer), "%.2f", f_pyranometer);
      sysevent_set(MB_PYRANOMETER_EVENT, s_pyranometer);
    }
  }
  return res;
}
#endif

#if (CONFIG_SENSOR_ATLAS_PH)
int read_ph(void) {
  int res = 0;
  char s_ph[10] = { 0 };

  res = atlas_ph_read(&dev, s_ph);

  if (res)
    LOGI(TAG, "Error executing atlas_ph_read(): %i", res);
  else
    sysevent_set(I2C_PH_EVENT, s_ph);

  return res;
}

int atlas_ph_cal_cmd(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1,2,3,4>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (atlas_ph_cal(&dev, type) != 0) {
    LOGI(TAG, "Failed to calibrate Atlas pH sensor = %d", type);
    return -1;
  }

  return 0;
}
#endif

#if (CONFIG_SENSOR_ATLAS_EC)
int read_ec(void) {
  int res = 0;
  char s_ec[10] = { 0 };

  res = atlas_ec_read(&dev, s_ec);

  if (res)
    LOGI(TAG, "Error executing atlas_ec_read(): %i", res);
  else
    sysevent_set(I2C_EC_EVENT, s_ec);

  return res;
}

int atlas_ec_cal_cmd(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1,2,3,4,5>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (atlas_ec_cal(&dev, type) != 0) {
    LOGI(TAG, "Failed to calibrate Atlas EC sensor = %d", type);
    return -1;
  }

  return 0;
}

int atlas_ec_probe_cmd(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1,2,3>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (atlas_ec_probe(&dev, type) != 0) {
    LOGI(TAG, "Failed to set probe of Atlas EC sensor = %d", type);
    return -1;
  }

  return 0;
}
#endif

#if (CONFIG_SENSOR_RK110_02)
int read_wind_direction(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_wind_direction[10] = { 0 };
  uint16_t u_wind_direction = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_wind_direction, value, 2);
      LOGI(TAG, "wind direction = %d", u_wind_direction);
      snprintf(s_wind_direction, sizeof(s_wind_direction), "%d", u_wind_direction);
      sysevent_set(MB_WIND_DIRECTION_EVENT, s_wind_direction);

      if (is_battery_model()) {
        LOGI(TAG, "wind direction = %d", u_wind_direction);
        // FLOGI(TAG, "wind direction = %d", u_wind_direction);
      } else {
        LOGI(TAG, "Power mode > wind direction = %d", u_wind_direction);
        // FLOGI(TAG, "Power mode > wind direction = %d", u_wind_direction);
      }
    }
  }
  return res;
}
#endif

#if (CONFIG_SENSOR_RK100_02)
int read_wind_speed(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_wind_speed[10] = { 0 };
  float f_wind_speed = 0.0;
  uint16_t u_wind_speed = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }
      memcpy(&u_wind_speed, value, 2);
      f_wind_speed = (float)(u_wind_speed / 10.0);
      LOGI(TAG, "wind speed = %.1f", f_wind_speed);
      snprintf(s_wind_speed, sizeof(s_wind_speed), "%.1f", f_wind_speed);
      sysevent_set(MB_WIND_SPEED_EVENT, s_wind_speed);

      if (is_battery_model()) {
        LOGI(TAG, "wind speed = %.1f", f_wind_speed);
        // FLOGI(TAG, "wind speed = %.1f", f_wind_speed);
      } else {
        LOGI(TAG, "Power mode > wind speed = %.1f", f_wind_speed);
        // FLOGI(TAG, "Power mode > wind speed = %.1f", f_wind_speed);
      }
    }
  }
  return res;
}
#endif

#if (CONFIG_SENSOR_RK500_13)
float convert_uint32_to_float(uint16_t low, uint16_t high) {
  float fValue = 0.0;
  uint32_t value = ((uint32_t)low << 16) | high;
  memcpy(&fValue, &value, sizeof(value));
  return fValue;
}

int read_rika_water_ec(void) {
  int res = 0;
  int data_len = 0;
  uint8_t value[30] = { 0 };
  char s_temperature[10] = { 0 };
  char s_ec[10] = { 0 };

  float f_ec = 0.0;
  float f_sal = 0.0;
  float f_temp = 0.0;

  uint16_t ec_low = 0, ec_high = 0;
  uint16_t temp_low = 0, temp_high = 0;
  uint16_t sal_low = 0, sal_high = 0;

  for (int i = 0; i < num_characteristic; i++) {
    if (mb_master_read_characteristic(mb_characteristic[i].cid, mb_characteristic[i].name, value, &data_len) == -1) {
      LOGE(TAG, "Failed to read value");
      res = -1;
    } else {
      for (int k = 0; k < data_len; k++) {
        LOGI(TAG, "value[%d] = [0x%x]", k, value[k]);
      }

      memcpy(&ec_low, value, 2);
      memcpy(&ec_high, value + 2, 2);
      memcpy(&temp_low, value + 8, 2);
      memcpy(&temp_high, value + 10, 2);
      memcpy(&sal_low, value + 16, 2);
      memcpy(&sal_high, value + 18, 2);
      f_ec = convert_uint32_to_float(ec_low, ec_high);
      f_sal = convert_uint32_to_float(sal_low, sal_high);
      f_temp = convert_uint32_to_float(temp_low, temp_high);
      LOGI(TAG, "ec = %.2f", f_ec);
      LOGI(TAG, "salinity = %.2f", f_sal);
      LOGI(TAG, "temperature = %.2f", f_temp);

      snprintf(s_temperature, sizeof(s_temperature), "%.2f", f_temp);
      snprintf(s_ec, sizeof(s_ec), "%.2f", f_ec);

      sysevent_set(MB_WATER_EC_EVENT, s_ec);
      sysevent_set(MB_TEMPERATURE_EVENT, s_temperature);

      if (is_battery_model()) {
        LOGI(TAG, "ec : %.2f, salinity : %.2f, temperature : %.2f", f_ec, f_sal, f_temp);
        // FLOGI(TAG, "ec : %.2f, salinity : %.2f, temperature : %.2f", f_ec, f_sal, f_temp);
      } else {
        LOGI(TAG, "Power mode > ec : %.2f, salinity : %.2f, temperature : %.2f", f_ec, f_sal, f_temp);
        // FLOGI(TAG, "Power mode > ec : %.2f, salinity : %.2f, temperature : %.2f", f_ec, f_sal, f_temp);
      }
    }
  }
  return res;
}
#endif

#if (SENSOR_TEST)
int read_test_data(void) {
  float random = 0;
  char buf[20] = { 0 };

  random = (float)(rand() % 10000) / 100;
  snprintf(buf, sizeof(buf), "%f", random);
  sysevent_set(I2C_TEMPERATURE_EVENT, buf);
  sysevent_set(I2C_HUMIDITY_EVENT, buf);
#if (CONFIG_SENSOR_SCD4X)
  sysevent_set(I2C_CO2_EVENT, buf);
#endif
  if (is_battery_model())
    sysevent_set(ADC_BATTERY_EVENT, buf);

  return 0;
}
#endif

int sensor_comparison(float m_sensor, float sensor, const float const_comparison) {
  LOGI(TAG, "sensor : %.2f, m_sensor : %.2f, diff : %.2f", sensor, m_sensor, fabs(sensor - m_sensor));
  if (fabs(sensor - m_sensor) >= const_comparison)
    return 1;

  return 0;
}

int sensor_read(void) {
  int rc = 0;
#if (SENSOR_TEST)
  rc = read_test_data();
  return rc;
#endif
#if (CONFIG_SENSOR_SHT3X)
  rc = read_temperature_humidity();
#elif (CONFIG_SENSOR_SCD4X)
  rc = read_co2_temperature_humidity();
#elif (CONFIG_SENSOR_RK520_02 || CONFIG_SENSOR_RS_ECTH)
  rc = read_soil_ec();
#elif (CONFIG_SENSOR_RK500_02)
  rc = read_water_ph();
#elif (CONFIG_SENSOR_SWSR7500)
  rc = read_solar_radiation();
#elif (CONFIG_SENSOR_ATLAS_PH)
  rc = read_ph();
#elif (CONFIG_SENSOR_ATLAS_EC)
  rc = read_ec();
#elif (CONFIG_SENSOR_RK110_02)
  rc = read_wind_direction();
#elif (CONFIG_SENSOR_RK100_02)
  rc = read_wind_speed();
#elif (CONFIG_SENSOR_RK500_13)
  rc = read_rika_water_ec();
#endif
  return rc;
}
