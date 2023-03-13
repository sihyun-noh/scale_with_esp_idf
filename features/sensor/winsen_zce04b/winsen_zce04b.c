#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "uart_api.h"
#include "winsen_zce04b.h"
#include "log.h"

#define BUF_SIZE 1024

static const char *TAG = "winsen_zce04b";

int winsen_zce04b_init(void) {
  int res = WINSEN_ZCE04B_OK;
  uint8_t set_config[9] = { 0xFF, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46 };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  res = uart_initialize(CONFIG_ZCE04B_UART_NUM, CONFIG_ZCE04B_UART_BAUDRATE, CONFIG_ZCE04B_UART_RXD, CONFIG_ZCE04B_UART_TXD,
                        CONFIG_ZCE04B_UART_RXBUF_SIZE, CONFIG_ZCE04B_UART_TXBUF_SIZE);

  if (res != WINSEN_ZCE04B_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    free(data);
    return WINSEN_ZCE04B_ERR_UART;
  }

  uart_write_data(CONFIG_ZCE04B_UART_NUM, set_config, sizeof(set_config));
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  uart_read_data(CONFIG_ZCE04B_UART_NUM, data, (BUF_SIZE - 1));
  free(data);

  return res;
}

int winsen_zce04b_read_manual(zce04b_data_t *data) {
  int len = 0;
  int res = WINSEN_ZCE04B_OK;
  uint8_t check_sum = 0;
  uint8_t petition[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
  uint8_t measure[20] = { 0x00 };

  uart_set_baud(CONFIG_ZCE04B_UART_NUM, CONFIG_ZCE04B_UART_BAUDRATE);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_write_data(CONFIG_ZCE04B_UART_NUM, petition, sizeof(petition));
  vTaskDelay(100 / portTICK_PERIOD_MS);

  len = uart_read_data(CONFIG_ZCE04B_UART_NUM, measure, 20);

  if ((measure[0] == 0xff) && (measure[1] == 0x86) && (len != 0)) {
    check_sum = winsen_zce04b_func_check_sum(measure, 11);
    if (check_sum != measure[10]) {
      LOGE(TAG, "Checksum Fail!!! (recv: 0x%x, calc: 0x%x)", measure[10], check_sum);
      res = WINSEN_ZCE04B_ERR_CHECKSUM;
    } else {
      data->co = measure[2] * 256 + measure[3];
      data->h2s = measure[4] * 256 + measure[5];
      data->o2 = (measure[6] * 256 + measure[7]) * 0.1;
      data->ch4 = measure[8] * 256 + measure[9];
    }
  } else {
    LOGE(TAG, "Invalid Data!!! (len: %d, start_byte: 0x%x, command: 0x%x)", len, measure[0], measure[1]);
    res = WINSEN_ZCE04B_ERR_STATUS;
  }

  return res;
}

uint8_t winsen_zce04b_func_check_sum(uint8_t *i, uint8_t ln) {
  uint8_t j, tempq = 0;
  i += 1;
  for (j = 0; j < (ln - 2); j++) {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;
  return (tempq);
}
