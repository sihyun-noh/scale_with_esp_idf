
// board_i2c.c
#include "board_i2c.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#define I2C_MASTER_SCL_IO         9
#define I2C_MASTER_SDA_IO         8
#define I2C_MASTER_NUM            0
#define I2C_MASTER_FREQ_HZ        400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

static bool s_i2c_inited = false;

esp_err_t board_i2c_init(void) {
  if (s_i2c_inited) {
    // 이미 초기화된 경우 그냥 OK
    return ESP_OK;
  }

  i2c_config_t conf = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_MASTER_SDA_IO,
    .scl_io_num = I2C_MASTER_SCL_IO,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
  if (err != ESP_OK) {
    return err;
  }

  err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
  if (err == ESP_ERR_INVALID_STATE) {
    // 이미 다른 곳에서 설치한 상태면 OK로 간주
    err = ESP_OK;
  }

  if (err == ESP_OK) {
    s_i2c_inited = true;
  }
  return err;
}
