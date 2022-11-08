/** @file i2c_hal.c
 *
 * @brief I2C HAL (peripheral abstraction api) that will be used in application layer
 *
 * This I2C api is designed to support various i2c drivers
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "i2c_hal.h"

#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/portmacro.h"
#include "hal/gpio_types.h"
#include "hal/i2c_types.h"

#define I2C_MASTER_FREQ_HZ 100000 /*!< I2C master clock frequency */
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TX_BUF_DISABLE 0

#define ACK_CHECK_EN 0x1  /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0 /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0       /*!< I2C ack value */
#define NACK_VAL 0x1      /*!< I2C nack value */

#define MAX_I2C_BUSES 2 /*!< ESP32 has 2 I2C bus */

typedef struct i2c_hal {
  int id;
  int ref_cnt;
  i2c_config_t config;
  SemaphoreHandle_t mutex;
} i2c_hal_t;

static i2c_hal_t i2c_hal_data[MAX_I2C_BUSES];

static i2c_hal_t *get_i2c_interface(int dev) {
  if (0 == i2c_hal_data[dev].ref_cnt) {
    return (i2c_hal_t *)NULL;
  }
  return &i2c_hal_data[dev];
}

int i2c_hal_init(int dev, int sda, int scl) {
  esp_err_t rc = ESP_FAIL;
  int bus = dev;

  if (bus >= 0 && bus < MAX_I2C_BUSES) {
    if (0 == i2c_hal_data[bus].ref_cnt) {
      i2c_hal_data[bus].mutex = xSemaphoreCreateMutex();
      if (i2c_hal_data[bus].mutex == NULL) {
        return HAL_I2C_INIT_ERR;
      }

      xSemaphoreTake(i2c_hal_data[bus].mutex, portMAX_DELAY);

      i2c_hal_data[bus].ref_cnt = 1;
      i2c_hal_data[bus].config.mode = I2C_MODE_MASTER;
      i2c_hal_data[bus].config.sda_pullup_en = GPIO_PULLUP_ENABLE;
      i2c_hal_data[bus].config.scl_pullup_en = GPIO_PULLUP_ENABLE;
      i2c_hal_data[bus].config.master.clk_speed = I2C_MASTER_FREQ_HZ;

      switch (bus) {
        case 0:
          i2c_hal_data[bus].id = I2C_NUM_0;
          i2c_hal_data[bus].config.sda_io_num = sda;
          i2c_hal_data[bus].config.scl_io_num = scl;
          break;
        case 1:
          i2c_hal_data[bus].id = I2C_NUM_1;
          i2c_hal_data[bus].config.sda_io_num = sda;
          i2c_hal_data[bus].config.scl_io_num = scl;
          break;
        default: break;
      }

      printf("i2c_driver bus : [%d]\n", i2c_hal_data[bus].id);
      printf("i2c_driver sda : [0x%x]\n", i2c_hal_data[bus].config.sda_io_num);
      printf("i2c_driver scl : [0x%x]\n", i2c_hal_data[bus].config.scl_io_num);

      rc = i2c_param_config(i2c_hal_data[bus].id, &i2c_hal_data[bus].config);
      rc = i2c_driver_install(i2c_hal_data[bus].id, i2c_hal_data[bus].config.mode, I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);

      printf("i2c_driver install : [%d]\n", rc);

      xSemaphoreGive(i2c_hal_data[bus].mutex);
    } else {
      i2c_hal_data[bus].ref_cnt++;
      rc = ESP_OK;
    }
  }

  if (ESP_OK == rc) {
    return HAL_I2C_NO_ERR;
  }

  return HAL_I2C_INIT_ERR;
}

int i2c_hal_free(int dev) {
  i2c_hal_t *hal = get_i2c_interface(dev);

  if (hal && --(hal->ref_cnt) <= 0) {
    if (hal->mutex) {
      vSemaphoreDelete(hal->mutex);
      hal->mutex = NULL;
    }
    i2c_driver_delete(hal->id);
  }

  return HAL_I2C_NO_ERR;
}

void i2c_hal_lock(int dev) {
  i2c_hal_t *hal = get_i2c_interface(dev);
  if (hal) {
    xSemaphoreTake(hal->mutex, portMAX_DELAY);
  }
}

void i2c_hal_unlock(int dev) {
  i2c_hal_t *hal = get_i2c_interface(dev);
  if (hal) {
    xSemaphoreGive(hal->mutex);
  }
}

int i2c_hal_read(int dev, uint16_t addr, uint8_t *data, size_t len) {
  /* Check for invalid arguments given */
  if (NULL == data || 0 == len) {
    return HAL_I2C_INVALID_ARGS;
  }

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ((uint8_t)addr << 1) | I2C_MASTER_READ, true);
  if (len > 1) {
    i2c_master_read(cmd, data, len - 1, ACK_VAL);
  }
  i2c_master_read_byte(cmd, data + len - 1, NACK_VAL);
  i2c_master_stop(cmd);
  esp_err_t rc = i2c_master_cmd_begin(dev, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  if (ESP_OK != rc) {
    return HAL_I2C_READ_ERR;
  }

  return HAL_I2C_NO_ERR;
}

int i2c_hal_write(int dev, uint16_t addr, const uint8_t *data, size_t len) {
  /* Check for invalid arguments given */
  if (NULL == data || 0 == len) {
    return HAL_I2C_INVALID_ARGS;
  }

  // printf("i2c_hal_write : dev = %d, addr = 0x%x\n", dev, addr);

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ((uint8_t)addr << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write(cmd, data, len, true);
  i2c_master_stop(cmd);
  esp_err_t rc = i2c_master_cmd_begin(dev, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  // printf("i2c_hal_write : rc = [%d]\n", rc);

  if (ESP_OK != rc) {
    return HAL_I2C_WRITE_ERR;
  }

  return HAL_I2C_NO_ERR;
}

int i2c_hal_write_regs(int dev, uint16_t addr, const void *reg, const uint8_t *data, size_t len) {
  /* Check for invalid arguments given */
  if (NULL == data || 0 == len) {
    return HAL_I2C_INVALID_ARGS;
  }

  // printf("i2c_hal_write_regs : dev = %d, addr = 0x%x\n", dev, addr);

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, ((uint8_t)addr << 1) | I2C_MASTER_WRITE, true);
  if (reg)
    i2c_master_write(cmd, (void *)reg, 1, true);
  i2c_master_write(cmd, data, len, true);
  i2c_master_stop(cmd);
  esp_err_t rc = i2c_master_cmd_begin(dev, cmd, 1000 / portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);

  // printf("i2c_hal_write_regs : rc = [%d]\n", rc);

  if (ESP_OK != rc) {
    return HAL_I2C_WRITE_ERR;
  }

  return HAL_I2C_NO_ERR;
}
