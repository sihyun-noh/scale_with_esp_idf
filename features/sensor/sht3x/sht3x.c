/** @file sht3x.c
 *
 * @brief Device driver module for Sensirion SHT3x Temperature and Humidity Sensors
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "i2c_api.h"
#include "sht3x.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"

#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#define DEBUG(...)            \
  do {                        \
    DEBUG_PRINT(__VA_ARGS__); \
  } while (0)
#define DEBUG_DEV(f, d, ...) \
  DEBUG("[SHT3x] %s dev=[%d], addr=[0x%02x] : " f "\n", __FUNCTION__, d->bus, d->addr, ##__VA_ARGS__)

#define CHECK_PARAM(conditon)                                                                       \
  do {                                                                                              \
    if (!(conditon)) {                                                                              \
      DEBUG("[SHT3x] %s: %s\n", __FUNCTION__, "parameter condition (" #conditon ") not fulfilled"); \
      return SHT3X_ERR_PARAMS;                                                                      \
    }                                                                                               \
  } while (0)

#define I2C_ACK_VAL 0x0
#define I2C_NACK_VAL 0x1

#define TASK_DELAY_MS(x) ((x) / portTICK_PERIOD_MS)
#define TASK_DELAY_SEC(x) ((x)*1000 / portTICK_PERIOD_MS)

/* SHT3x common commands */
#define SHT3X_STATUS_CMD 0xF32D       /* Get status command */
#define SHT3X_CLEAR_STATUS_CMD 0x3041 /* Clear status command */
#define SHT3X_BREAK_CMD 0x3093        /* Break command */
#define SHT3X_RESET_CMD 0x30A2        /* Reset command */
#define SHT3X_FETCH_DATA_CMD 0xE000   /* Fetch data command */
#define SHT3X_HEATER_OFF_CMD 0x3066   /* Heater disable command */

/* SHT3x status register flags */
/* Refer to 4.11 Status Register in SHT3x datasheet */
/* Status register bit mask : Table 17 */
#define SHT3X_STATUS_REG_MASK (0xac13) /* valid status register bit mask */
#define SHT3X_STATUS_REG_CRC (1 << 0)  /* write data checksum status */
#define SHT3X_STATUS_REG_CMD (1 << 1)  /* last command execution status */

#define TIME_TO_TICKS(ms) ((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)

/* Maximum measurement durations dependent on repeatability in ms */
#define SHT3X_MEAS_DURATION_REP_HIGH 15
#define SHT3X_MEAS_DURATION_REP_MEDIUM 6
#define SHT3X_MEAS_DURATION_REP_LOW 4

/* Measurement durations in us */
const uint16_t SHT3X_MEAS_DURATION_US[3] = { SHT3X_MEAS_DURATION_REP_HIGH * 1000, SHT3X_MEAS_DURATION_REP_MEDIUM * 1000,
                                             SHT3X_MEAS_DURATION_REP_LOW * 1000 };

// Measurement durations in RTOS ticks
const uint16_t SHT3X_MEAS_DURATION_TICKS[3] = { TIME_TO_TICKS(SHT3X_MEAS_DURATION_REP_HIGH),
                                                TIME_TO_TICKS(SHT3X_MEAS_DURATION_REP_MEDIUM),
                                                TIME_TO_TICKS(SHT3X_MEAS_DURATION_REP_LOW) };

static const uint32_t SHT3X_MEASURE_PERIOD[] = {
  0,    /* SINGLE_SHOT */
  2000, /* PERIODIC_05 */
  1000, /* PERIODIC_1 */
  500,  /* PERIODIC_2 */
  250,  /* PERIODIC_4 */
  100   /* PERIODIC_10 */
};

/* SHT3x measurement command sequence */
static const uint16_t SHT3X_MEASURE_CMD[6][3] = {
  { 0x2400, 0x240b, 0x2416 },  // [SINGLE_SHOT][H,M,L] without clock stretching
  { 0x2032, 0x2024, 0x202f },  // [PERIODIC_05][H,M,L]
  { 0x2130, 0x2126, 0x212d },  // [PERIODIC_1 ][H,M,L]
  { 0x2236, 0x2220, 0x222b },  // [PERIODIC_2 ][H,M,L]
  { 0x2234, 0x2322, 0x2329 },  // [PERIODIC_4 ][H,M,L]
  { 0x2737, 0x2721, 0x272a }   // [PERIODIC_10][H,M,L]
};

#define SHT3X_RAW_DATA_SIZE 6

/* Functions for internal use */
static int get_raw_data(sht3x_dev_t* dev, uint8_t* raw_mode);
static int compute_values(uint8_t* raw_data, float* temperature, float* humidity);

/* Sensor commands */
static int start_measurement(sht3x_dev_t* dev);
static int reset(sht3x_dev_t* dev);
static int get_status(sht3x_dev_t* dev, uint16_t* status);

/* Sensor access(write/read command) functions */
static int send_command(sht3x_dev_t* dev, uint16_t cmd);
static int read_data(sht3x_dev_t* dev, uint8_t* data, uint8_t len);

const uint8_t g_polynom = 0x31;
static uint8_t crc8(uint8_t* data, size_t len) {
  // initialization value
  uint8_t crc = 0xff;

  // iterate over all bytes
  for (int i = 0; i < len; i++) {
    crc ^= data[i];

    for (int i = 0; i < 8; i++) {
      bool xor = crc & 0x80;
      crc = crc << 1;
      crc = xor? crc ^ g_polynom : crc;
    }
  }

  return crc;
}

/* Public APIs */

#define DEV_I2C (dev->bus)
#define DEV_ADDR (dev->addr)
#define DEV_I2C_SDA (dev->sda_pin)
#define DEV_I2C_SCL (dev->scl_pin)

int sht3x_init(sht3x_dev_t* dev, const sht3x_params_t* params) {
  CHECK_PARAM(dev != NULL);
  CHECK_PARAM(params != NULL);

  int res = SHT3X_OK;
  uint16_t status;

  /* Initialize sensor data structure */
  dev->bus = params->bus;
  dev->addr = params->addr;
  dev->sda_pin = params->sda_pin;
  dev->scl_pin = params->scl_pin;
  dev->mode = params->mode;
  dev->repeat = params->repeat;

  dev->meas_started = false;
  dev->meas_start_time = 0;
  dev->meas_duration = 0;

  /* TODO : Need to initialize the i2c driver */
  if ((res = i2c_init(DEV_I2C, DEV_I2C_SDA, DEV_I2C_SCL)) != 0) {
    DEBUG("Could not initialize, error = %d\n", res);
    return SHT3X_ERR_I2C;
  }

  /* Try to reset the sensor */
  if ((res = reset(dev)) != SHT3X_OK) {
    DEBUG("Could not reset the sensor\n");
  }

  /* Check again the status after clear status command */
  if ((res = get_status(dev, &status)) != SHT3X_OK) {
    DEBUG("Could not get sensor status\n");
    return res;
  }

  DEBUG("Sensor Initialize success\n");
  return res;
}

int sht3x_read(sht3x_dev_t* dev, float* temperature, float* humidity) {
  CHECK_PARAM(dev != NULL);
  CHECK_PARAM(temperature != NULL || humidity != NULL);

  int res = SHT3X_OK;

  uint8_t raw_data[SHT3X_RAW_DATA_SIZE];

  /*
   * fetches raw sensor data, implicitly starts measurement if necessary,
   * In single-shot measurement mode, and waits until sensor data are available
   */
  if ((res = get_raw_data(dev, raw_data)) != SHT3X_OK) {
    return res;
  }

  return compute_values(raw_data, temperature, humidity);
}

/* Functions for internal use only */

static int start_measurement(sht3x_dev_t* dev) {
  /* Start measurment according to selected mode */
  if (send_command(dev, SHT3X_MEASURE_CMD[dev->mode][dev->repeat]) != SHT3X_OK) {
    DEBUG_DEV("%s", dev, "Start measurement failed, Could not send SHT3X_MEASURE_CMD to sensor");
    return SHT3X_ERR_I2C;
  }

  /*
   * In periodic mode, check whether the command were processed,
   * In signle shot mode, reading result has to follow directly.
   */
  if (dev->mode != SHT3X_SINGLE_SHOT) {
    /* Sensor needs up to max 15 ms to process the measurement command
     * Refer to the 2.2 Timing Specification in SHT3x-DIS datasheet for the Sensor system.
     */
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Wait 20 ms for processing the command.

    uint16_t status;
    int res;

    /* Read the status after measurment command */
    if ((res = get_status(dev, &status)) != SHT3X_OK) {
      return res;
    }

    if (status & SHT3X_STATUS_REG_CMD) {
      DEBUG_DEV("%s", dev, "Start measurment failed, SHT3X_CMD_MEASURE were not executed");
      return SHT3X_ERR_MEASURE_CMD;
    }
  }

  dev->meas_start_time = xTaskGetTickCount();
  dev->meas_duration = SHT3X_MEAS_DURATION_TICKS[dev->repeat];
  dev->meas_started = true;

  return SHT3X_OK;
}

static int get_raw_data(sht3x_dev_t* dev, uint8_t* raw_data) {
  CHECK_PARAM(dev != NULL);
  CHECK_PARAM(raw_data != NULL);

  int res = SHT3X_OK;

  /* Start measurement if it has not stareted yet */
  if ((false == dev->meas_started) && (res = start_measurement(dev)) != SHT3X_OK) {
    return res;
  }

  /* Set the time eplased since the start of current measurement cycle */
  uint32_t elapse_time = xTaskGetTickCount() - dev->meas_start_time;
  if (elapse_time < dev->meas_duration) {
    /* if necessary, wait until the measurement results become available */
    vTaskDelay(TASK_DELAY_MS(dev->meas_duration - elapse_time));
  }

  /* Send fetch command in any periodic mode before read raw data */
  if ((dev->mode != SHT3X_SINGLE_SHOT) && (send_command(dev, SHT3X_FETCH_DATA_CMD) != SHT3X_OK)) {
    DEBUG_DEV("%s", dev, "Could not send SHT3X_FETCH_DATA_CMD to sensor");
    return SHT3X_ERR_I2C;
  }

  /* Read raw data */
  if (read_data(dev, raw_data, SHT3X_RAW_DATA_SIZE) != SHT3X_OK) {
    DEBUG_DEV("%s", dev, "Could not read raw sensor data");
    return SHT3X_ERR_I2C;
  }

  /* In single shot mode update measurement started flag of the driver */
  if (dev->mode == SHT3X_SINGLE_SHOT) {
    dev->meas_started = false;
  } else {
    dev->meas_start_time = xTaskGetTickCount();
    dev->meas_duration = SHT3X_MEASURE_PERIOD[dev->mode];
  }

  /* Check temperature crc */
  if (crc8(raw_data, 2) != raw_data[2]) {
    DEBUG_DEV("%s", dev, "CRC check failed for temperature data");
    return SHT3X_ERR_CRC;
  }

  /* Check humidity crc */
  if (crc8(raw_data + 3, 2) != raw_data[5]) {
    DEBUG_DEV("%s", dev, "CRC check failed for humidity data");
    return SHT3X_ERR_CRC;
  }

  return SHT3X_OK;
}

static int compute_values(uint8_t* raw_data, float* temperature, float* humidity) {
  if (temperature) {
    *temperature = ((((raw_data[0] * 256.0) + raw_data[1]) * 175) / 65535.0) - 45;
  }

  if (humidity) {
    *humidity = ((((raw_data[3] * 256.0) + raw_data[4]) * 100) / 65535.0);
  }
  return SHT3X_OK;
}

static int send_command(sht3x_dev_t* dev, uint16_t cmd) {
  CHECK_PARAM(dev != NULL);

  int res;

  uint8_t data[2] = { cmd >> 8, cmd & 0xff };

  DEBUG_DEV("Send command MSB=[0x%02x] LSB=[0x%02x]", dev, data[0], data[1]);

  i2c_lock(dev->bus);
  res = i2c_write_bytes(dev->bus, dev->addr, data, 2, 0);
  i2c_unlock(dev->bus);

  if (res != 0) {
    DEBUG_DEV("Could not send command [0x%02x][0x%02x] to sensor, error = [%d]", dev, data[0], data[1], res);
    return SHT3X_ERR_I2C;
  }

  return SHT3X_OK;
}

static int read_data(sht3x_dev_t* dev, uint8_t* data, uint8_t len) {
  int res;

  i2c_lock(dev->bus);
  res = i2c_read_bytes(dev->bus, dev->addr, data, len, 0);
  i2c_unlock(dev->bus);

  if (res != 0) {
    DEBUG_DEV("Could not read %d bytes from sensor, error = [%d]", dev, len, res);
    return SHT3X_ERR_I2C;
  }

  return SHT3X_OK;
}

static int reset(sht3x_dev_t* dev) {
  int res = SHT3X_OK;
  CHECK_PARAM(dev != NULL);

  /* Send soft-reset command */
  if (send_command(dev, SHT3X_RESET_CMD) != SHT3X_OK) {
    return SHT3X_ERR_I2C;
  }

  /* Wait for 2 ms, the time needed to restart (according to datasheet 0.5 ms) */
  vTaskDelay(TASK_DELAY_MS(100));

  return res;
}

static int get_status(sht3x_dev_t* dev, uint16_t* status) {
  CHECK_PARAM(dev != NULL);
  CHECK_PARAM(status != NULL);

  uint8_t data[3];

  /* Read sensor status */
  if (send_command(dev, SHT3X_STATUS_CMD) != SHT3X_OK) {
    return SHT3X_ERR_I2C;
  }

  if (read_data(dev, data, 3) != SHT3X_OK) {
    return SHT3X_ERR_I2C;
  }

  /* Check status crc */
  if (crc8(data, 2) != data[2]) {
    return SHT3X_ERR_CRC;
  }

  *status = (data[0] << 8 | data[1]) & SHT3X_STATUS_REG_MASK;
  DEBUG("status = [0x%02x]\n", *status);

  return SHT3X_OK;
}
