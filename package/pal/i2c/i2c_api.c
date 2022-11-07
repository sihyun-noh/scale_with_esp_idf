/** @file i2c_api.c
 *
 * @brief I2C peripheral abstraction api that will be used in application layer
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

#include "i2c_api.h"
#include "esp32/i2c_hal.h"

int i2c_init(int dev, int sda, int scl) {
  return i2c_hal_init(dev, sda, scl);
}

void i2c_lock(int dev) {
  i2c_hal_lock(dev);
}

void i2c_unlock(int dev) {
  i2c_hal_unlock(dev);
}

int i2c_read_reg(int dev, uint16_t addr, uint8_t reg, uint8_t *data, uint8_t flags) {
  return i2c_read_regs(dev, addr, reg, data, 1, flags);
}

/* Need to support the register with 10 and 16 bits */
int i2c_read_regs(int dev, uint16_t addr, uint8_t reg, uint8_t *data, size_t len, uint8_t flags) {
  /* First set addr and register */
  int ret = i2c_write_bytes(dev, addr, &reg, 1, flags);
  if (ret < 0) {
    return ret;
  }

  /* Then get the data from device */
  return i2c_read_bytes(dev, addr, data, len, flags);
}

int i2c_read_byte(int dev, uint16_t addr, uint8_t *data, uint8_t flags) {
  return i2c_read_bytes(dev, addr, data, 1, flags);
}

int i2c_read_bytes(int dev, uint16_t addr, uint8_t *data, size_t len, uint8_t flags) {
  return i2c_hal_read(dev, addr, data, len);
}

int i2c_write_reg(int dev, uint16_t addr, uint8_t reg, uint8_t data, uint8_t flags) {
  return i2c_write_regs(dev, addr, reg, &data, 1, flags);
}

/* Need to support the register with 10 and 16 bits */
int i2c_write_regs(int dev, uint16_t addr, uint8_t reg, const uint8_t *data, size_t len, uint8_t flags) {
  return i2c_hal_write_regs(dev, addr, &reg, data, len);
}

int i2c_write_byte(int dev, uint16_t addr, uint8_t data, uint8_t flags) {
  return i2c_write_bytes(dev, addr, &data, 1, flags);
}

int i2c_write_bytes(int dev, uint16_t addr, const uint8_t *data, size_t len, uint8_t flags) {
  return i2c_hal_write(dev, addr, data, len);
}
