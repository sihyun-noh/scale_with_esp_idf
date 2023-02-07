/** @file scd4x.c
 *
 * @brief Device driver module for Sensirion SCD4x Co2 and Temperature and Humidity Sensors
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
#include "scd4x.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"
#include "log.h"

/**
 * @brief Wake up the sensor from sleep mode
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
static int scd4x_wake_up(scd4x_dev_t* dev);

/**
 * @brief stop periodic measurement to change the sensor configuration or to save power
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
static int scd4x_stop_periodic_measurement(scd4x_dev_t* dev);

/**
 * @brief The reinit command reinitializes the sensor by reloading user settings from EEPROM
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
static int scd4x_reinit(scd4x_dev_t* dev);

/**
 * @brief Reading out the serial number can be used to identify the chip
 * and to verify the presence of the sensor
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
static int scd4x_get_serial_number(scd4x_dev_t* dev);

/**
 * @brief start periodic measurement, signal update interval is 5 seconds
 *
 * @param[in]   dev     Device context of SCD4x device to be initialized
 *
 * @return 0 on success , otherwise negative error code
 */
static int scd4x_start_periodic_measurement(scd4x_dev_t* dev);

/**
 * @brief crc check func
 *
 * @param[in]   data    crc check data
 * @param[in]   len     data length
 *
 *
 * @return 0 on success , otherwise negative error code
 */
static uint8_t crc8(uint8_t* data, size_t len);

static const char* TAG = "scd4x";

int scd4x_init(scd4x_dev_t* dev, const scd4x_params_t* params) {
  int ret = 0;

  dev->bus = params->bus;
  dev->addr = params->addr;
  dev->sda_pin = params->sda_pin;
  dev->scl_pin = params->scl_pin;

  if ((ret = i2c_init(dev->bus, dev->sda_pin, dev->scl_pin)) != 0) {
    LOGI(TAG, "Could not initialize, error = %d\n", ret);
    goto _exit;
  }

  // Clean up potential SCD40 states
  scd4x_wake_up(dev);
  scd4x_stop_periodic_measurement(dev);
  scd4x_reinit(dev);

  if ((ret = scd4x_get_serial_number(dev)) != 0) {
    LOGI(TAG, "Error executing scd4x get serial number(): %i", ret);
    goto _exit;
  } else {
    LOGI(TAG, "serial: 0x%04x%04x%04x\n", dev->serial[0], dev->serial[1], dev->serial[2]);
  }

  // Start Measurement
  if ((ret = scd4x_start_periodic_measurement(dev)) != 0) {
    LOGI(TAG, "Error executing scd4x get serial number(): %i", ret);
  }

_exit:
  return ret;
}

static int scd4x_wake_up(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x36, 0xF6 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  LOGI(TAG, "scd4x wake up");
  vTaskDelay(20 / portTICK_PERIOD_MS);

  return ret;
}

static int scd4x_stop_periodic_measurement(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x3F, 0x86 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);
  LOGI(TAG, "scd4x stop periodic measurement");
  vTaskDelay(500 / portTICK_PERIOD_MS);

  return ret;
}

static int scd4x_reinit(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x36, 0x46 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);
  LOGI(TAG, "scd4x reinit");
  vTaskDelay(20 / portTICK_PERIOD_MS);

  return ret;
}

static int scd4x_get_serial_number(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x36, 0x82 };
  uint8_t read[9] = { 0 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  LOGI(TAG, "scd4x get serial number");
  vTaskDelay(20 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  /* Check crc */
  for (uint i = 0; i < sizeof(read); i += 3) {
    if (crc8(&read[i], 2) != read[i + 2]) {
      LOGI(TAG, "CRC check failed for serial data");
      ret = -1;
      break;
    }
  }

  if (ret == 0) {
    dev->serial[0] = (uint16_t)read[0] << 8 | (uint16_t)read[1];
    dev->serial[1] = (uint16_t)read[3] << 8 | (uint16_t)read[4];
    dev->serial[2] = (uint16_t)read[6] << 8 | (uint16_t)read[7];
  }

  return ret;
}

static int scd4x_start_periodic_measurement(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x21, 0xB1 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);
  LOGI(TAG, "scd4x start periodic measurement");
  vTaskDelay(1 / portTICK_PERIOD_MS);

  return ret;
}

int scd4x_get_data_ready_flag(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0xE4, 0xB8 };
  uint8_t read[3] = { 0 };
  uint16_t local_data_ready = 0;

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  vTaskDelay(1 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  /* Check crc */
  if (crc8(read, 2) != read[2]) {
    LOGI(TAG, "CRC check failed for serial data");
    ret = -1;
  }

  if (ret == 0) {
    local_data_ready = (uint16_t)read[0] << 8 | (uint16_t)read[1];
    ret = (local_data_ready & 0x7FF) != 0;
  }

  return ret;
}

int scd4x_read_measurement(scd4x_dev_t* dev, uint16_t* co2, float* temperature_deg_c, float* humidity_percent_rh) {
  int ret = 0;
  uint8_t data[2] = { 0xEC, 0x05 };
  uint8_t read[9] = { 0 };
  uint16_t temperature;
  uint16_t humidity;

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  vTaskDelay(1 / portTICK_PERIOD_MS);
  if ((ret = i2c_read_bytes(dev->bus, dev->addr, read, sizeof(read), 0)) != 0) {
    LOGI(TAG, "Error i2c read bytes %i", ret);
  }
  i2c_unlock(dev->bus);

  /* Check crc */
  for (uint i = 0; i < sizeof(read); i += 3) {
    if (crc8(&read[i], 2) != read[i + 2]) {
      LOGI(TAG, "CRC check failed for serial data");
      ret = -1;
      break;
    }
  }

  if (ret == 0) {
    *co2 = (uint16_t)read[0] << 8 | (uint16_t)read[1];
    temperature = (uint16_t)read[3] << 8 | (uint16_t)read[4];
    humidity = (uint16_t)read[6] << 8 | (uint16_t)read[7];

    *temperature_deg_c = (float)temperature * 175.0f / 65536.0f - 45.0f;
    *humidity_percent_rh = (float)humidity * 100.0f / 65536.0f;
  }

  return ret;
}

int scd4x_start_low_power_periodic_measurement(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x21, 0xAC };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);
  LOGI(TAG, "scd4x start low power periodic measurement");

  return ret;
}

int scd4x_power_down(scd4x_dev_t* dev) {
  int ret = 0;
  uint8_t data[2] = { 0x36, 0xE0 };

  i2c_lock(dev->bus);
  if ((ret = i2c_write_bytes(dev->bus, dev->addr, data, sizeof(data), 0)) != 0) {
    LOGI(TAG, "Error i2c write bytes %i", ret);
  }
  i2c_unlock(dev->bus);
  LOGI(TAG, "scd4x power down");
  vTaskDelay(1 / portTICK_PERIOD_MS);

  return ret;
}

static const uint8_t g_polynom = 0x31;
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
