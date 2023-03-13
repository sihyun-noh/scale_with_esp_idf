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
  uint8_t set_config[9] = { 0xFF, 0x01, 0x78, 0x04, 0x00, 0x00, 0x00, 0x00, 0x83 };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  res = uart_initialize(CONFIG_ZE03_UART_NUM, CONFIG_ZE03_UART_BAUDRATE, CONFIG_ZE03_UART_RXD, CONFIG_ZE03_UART_TXD,
                        CONFIG_ZE03_UART_RXBUF_SIZE, CONFIG_ZE03_UART_TXBUF_SIZE);

  if (res != WINSEN_ZE03_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    free(data);
    return WINSEN_ZE03_ERR_UART;
  }

  uart_write_data(CONFIG_ZE03_UART_NUM, set_config, sizeof(set_config));
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  uart_read_data(CONFIG_ZE03_UART_NUM, data, (BUF_SIZE - 1));
  free(data);

  return res;
}

int winsen_ze03_read_manual(int *data) {
  int len = 0;
  int res = WINSEN_ZE03_OK;
  uint8_t check_sum = 0;
  uint8_t petition[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
  uint8_t measure[20] = { 0x00 };

  uart_set_baud(CONFIG_ZE03_UART_NUM, CONFIG_ZE03_UART_BAUDRATE);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_write_data(CONFIG_ZE03_UART_NUM, petition, sizeof(petition));
  vTaskDelay(100 / portTICK_PERIOD_MS);

  len = uart_read_data(CONFIG_ZE03_UART_NUM, measure, 20);

  if ((measure[0] == 0xff) && (measure[1] == 0x86) && (len != 0)) {
    check_sum = winsen_ze03_func_check_sum(measure, 9);
    if (check_sum != measure[8]) {
      LOGE(TAG, "Checksum Fail!!! (recv: 0x%x, calc: 0x%x)", measure[8], check_sum);
      res = WINSEN_ZE03_ERR_CHECKSUM;
    } else {
      *data = measure[2] * 256 + measure[3];
    }
  } else {
    LOGE(TAG, "Invalid Data!!! (len: %d, start_byte: 0x%x, command: 0x%x)", len, measure[0], measure[1]);
    res = WINSEN_ZE03_ERR_STATUS;
  }

  return res;
}

uint8_t winsen_ze03_func_check_sum(uint8_t *i, uint8_t ln) {
  uint8_t j, tempq = 0;

  i += 1;
  for (j = 0; j < (ln - 2); j++) {
    tempq += *i;
    i++;
  }
  tempq = (~tempq) + 1;

  return (tempq);
}
