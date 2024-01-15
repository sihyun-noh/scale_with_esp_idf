/*
  Apply to CAS WTM-500 Indicator 
  Apply "3" to setting mode F2-6(data format mode)
  Apply "4" to setting mode F2-7(RS485 - Output mode)
*/


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "log.h"
#include "sys_status.h"
#include "main.h"
#include "filelog.h"
#include "config.h"
#include "gpio_api.h"
#include "uart_api.h"
#include "espnow.h"

#define BUF_SIZE 23

typedef enum {
  WINSEN_ZE03 = 0x01,
  WINSEN_ZCE04B = 0x02,
  TH01 = 0x03,
} UART_MUX_ADDR;
typedef enum {
  WINSEN_ZE03_OK = 0,
  WINSEN_ZE03_ERR_UART = -1,
  WINSEN_ZE03_ERR_CHECKSUM = -2,
  WINSEN_ZE03_ERR_STATUS = -3,
  WINSEN_ZE03_ERR_PARAMS = -4,
} winsen_ze03_err_code;


static const char *TAG = "weight_task";

int wetght_uart_485_init(void){

  int res = WINSEN_ZE03_OK;
  uint8_t set_config[6] = {0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A}; // ID 01번
  //uint8_t set_config[6] = {0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A}; // 무게값 요청
  //uint8_t set_config[6] = {0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A}; // 무게값 요청
  //uint8_t set_config[6] = {0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A}; // 무게값 요청

  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  cas_22byte_format_t read_weight_data;
 
  res = uart_initialize(UART_PORT_NUM, BAUD_RATE, MB_RX_PIN, MB_TX_PIN, UART_RXBUF_SIZE, UART_TXBUF_SIZE);

  if (res != WINSEN_ZE03_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    free(data);
    return WINSEN_ZE03_ERR_UART;
  }

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  LOGI(TAG, "uart 485 read data = %s", data);
  LOG_BUFFER_HEXDUMP(TAG, &data, sizeof(data), LOG_INFO);

  memset(&read_weight_data, 0x00, sizeof(read_weight_data));

  memcpy(read_weight_data.states, data, 2);
  memcpy(read_weight_data.measurement_states, data+3, 2);
  memcpy(read_weight_data.lamp_states, data+7, 1);
  memcpy(read_weight_data.data, data+9, 8);
  memcpy(read_weight_data.relay, data+17, 1);
  memcpy(read_weight_data.unit, data+18, 2);

  LOG_BUFFER_HEXDUMP(TAG, &read_weight_data, sizeof(read_weight_data), LOG_INFO);

  // LOGI(TAG, "uart 485 read states = %s\n", read_weight_data.states);
  // LOGI(TAG, "uart 485 read measurement = %s\n", read_weight_data.measurement_states);
  // LOGI(TAG, "uart 485 read data = %s\n", read_weight_data.data);
   
  free(data);

  return res;

}


cas_22byte_format_t *read_weight_value(void){

  uint8_t set_config[6] = {0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A};
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static cas_22byte_format_t read_weight_data;

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));

  //memset(&read_weight_data, 0x00, sizeof(read_weight_data));

  memcpy(read_weight_data.states, data, 2);
  memcpy(read_weight_data.measurement_states, data+3, 2);
  memcpy(read_weight_data.lamp_states, data+7, 1);
  memcpy(read_weight_data.data, data+9, 8);
  memcpy(read_weight_data.relay, data+17, 1);
  memcpy(read_weight_data.unit, data+18, 2);

 // LOG_BUFFER_HEXDUMP(TAG, &read_weight_data, sizeof(read_weight_data), LOG_INFO); 
  free(data);

  return &read_weight_data;
}

int cas_zero_command(){
  uint8_t set_config[6] = {0x30, 0x31, 0x4D, 0x5A, 0x0D, 0x0A};
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  char buff[2];

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));

  LOGI(TAG, "uart 485 read data = %s", data);
  
  memset(&buff, 0x00, sizeof(buff));
  memcpy(buff, data+2, 2);
  if(strncmp(buff,"MZ",2)== 0){
    LOGI(TAG, "zero set success!!");
    return 0;
  }else{
    LOGI(TAG, "zero set fail!!");
    return -1;
  }

}

