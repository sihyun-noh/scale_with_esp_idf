#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "uart_api.h"
#include "th01.h"
#include "log.h"

#define BUF_SIZE 1024

static const char *TAG = "th01";

int th01_init(void) {
  int res = TH01_OK;
  uint8_t set_config[6] = { 0x24, 0x30, 0x31, 0x54, 0x0D, 0x0A };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);

  res = uart_initialize(CONFIG_TH01_UART_NUM, CONFIG_TH01_UART_BAUDRATE, CONFIG_TH01_UART_RXD, CONFIG_TH01_UART_TXD,
                        CONFIG_TH01_UART_RXBUF_SIZE, CONFIG_TH01_UART_TXBUF_SIZE);

  if (res != TH01_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    free(data);
    return TH01_ERR_UART;
  }

  uart_write_data(CONFIG_TH01_UART_NUM, set_config, sizeof(set_config));
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  uart_read_data(CONFIG_TH01_UART_NUM, data, (BUF_SIZE - 1));
  free(data);

  return res;
}

int th01_read_manual(th01_data_t *data) {
  int len = 0;
  int res = TH01_OK;
  uint8_t petition[6] = { 0x24, 0x30, 0x31, 0x54, 0x0D, 0x0A };
  uint8_t measure[40] = { 0x00 };
  float buf_t1 = 999;
  float buf_h = 999;

  uart_set_baud(CONFIG_TH01_UART_NUM, CONFIG_TH01_UART_BAUDRATE);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_write_data(CONFIG_TH01_UART_NUM, petition, sizeof(petition));
  vTaskDelay(100 / portTICK_PERIOD_MS);

  len = uart_read_data(CONFIG_TH01_UART_NUM, measure, 40);

  if ((measure[0] == 0x23) && (measure[1] == 0x30) && (len != 0)) {
    for (int i = 0; i < len; i++) {
      if (measure[i] == '=') {
        if (buf_t1 == 999) {
          buf_t1 = ((float)atof((char *)&measure[i+1]))*10;
          data->temp = buf_t1 / 10.0;
        } else if (buf_h == 999) {
          buf_h = ((float)atof((char *)&measure[i+1]))*10;
          data->mos = buf_h / 10.0;
        }
      }
      if (measure[i] == 0x0a)
        break;
    }
  } else {
    LOGE(TAG, "Invalid Data!!! (len: %d, start_byte: 0x%x, command: 0x%x)", len, measure[0], measure[1]);
    res = TH01_ERR_STATUS;
  }

  return res;
}
