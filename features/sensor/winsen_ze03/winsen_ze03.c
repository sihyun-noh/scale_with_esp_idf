#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "uart_api.h"
#include "winsen_ze03.h"
#include "log.h"

#define BUF_SIZE 1024

static const char *TAG = "winsen_ze03";

int winsen_ze03_init(void) {
  int res = WINSEN_ZE03_OK;

  res = uart_initialize(CONFIG_ZE03_UART_NUM, CONFIG_ZE03_UART_BAUDRATE, CONFIG_ZE03_UART_RXD, CONFIG_ZE03_UART_TXD,
                        CONFIG_ZE03_UART_RXBUF_SIZE, CONFIG_ZE03_UART_TXBUF_SIZE);

  if (res != WINSEN_ZE03_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    return WINSEN_ZE03_ERR_UART;
  }

  return res;
}

void winsen_ze03_set_active(bool active) {
  uint8_t set_config[9] = { 0xFF, 0x01, 0x78, 0x04, 0x00, 0x00, 0x00, 0x00, 0x83 };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  if (active) {
    set_config[3] = 0x03;
    set_config[8] = 0x84;
  }
  uart_write_data(CONFIG_ZE03_UART_NUM, set_config, sizeof(set_config));
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  uart_read_data(CONFIG_ZE03_UART_NUM, data, (BUF_SIZE - 1));
  free(data);
}

float winsen_ze03_read_manual(void) {
  float ppm;

  int len = 0;
  uint8_t check_sum = 0;
  uint8_t petition[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
  uint8_t measure[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

  uart_write_data(CONFIG_ZE03_UART_NUM, petition, sizeof(petition));
  vTaskDelay(1500 / portTICK_PERIOD_MS);

  len = uart_read_data(CONFIG_ZE03_UART_NUM, measure, 9);

  LOGI(TAG, "(%d) 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", len, measure[0], measure[1], measure[2], measure[3],
       measure[4], measure[5], measure[6], measure[7], measure[8]);
  if (measure[0] == 0xff && measure[1] == 0x86) {
    ppm = measure[2] * 256 + measure[3];
    check_sum = func_check_sum(measure, 9);
    if (check_sum != measure[8]) {
      LOGI(TAG, "Checksum Fail!!! (recv: 0x%x, calc: 0x%x)", measure[8], check_sum);
      ppm = -1;
    }
  } else {
    ppm = -1;
  }
  LOGI(TAG, "%f (ppm)", ppm);
  return ppm;
}

uint8_t func_check_sum(uint8_t *i, uint8_t ln) {
  uint8_t j, tempq = 0;
  i += 1;
  for (j = 0; j < (ln - 2); j++) {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;
  return (tempq);
}
