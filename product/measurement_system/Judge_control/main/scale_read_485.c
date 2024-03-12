/*
  Apply to CAS WTM-500 Indicator
  Apply "3" to setting mode F2-6(data format mode)
  Apply "4" to setting mode F2-7(RS485 - Output mode)
*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "log.h"
#include "sys_status.h"
#include "main.h"
#include "filelog.h"
#include "scale_read_485.h"
#include "gpio_api.h"
#include "uart_api.h"
#include "config.h"

// #define BUF_SIZE 23
#define BUF_SIZE 255
#define INDICATOR_DEBUG 0

typedef enum {
  TASK_OK = 0,
  TASK_ERROR = -1,
} winsen_ze03_err_code;

static const char *TAG = "scale_read_485_TASK";

int weight_uart_485_init(void) {
  int res = TASK_OK;
  res = uart_initialize(UART_PORT_NUM, BAUD_RATE, MB_RX_PIN, MB_TX_PIN, UART_RXBUF_SIZE, UART_TXBUF_SIZE);
  if (res != TASK_OK) {
    LOGE(TAG, "Could not initialize, error = %d", res);
    return TASK_ERROR;
  } else {
    LOGI(TAG, "Uart initialize success = %d", res);
  }
  return res;
}

/*
WTM Command : mode 1
format : CAS 22 Byte
structure (ASCII)
 --------------------------------------------------------------------------------------------------------------------------
 |  U  |  S  |  '\0' |  G  |  S  |  '\0'  | 0xff | 0xff |     8byte data  |      0xff    |    k    |     g       | CR |
LF | US(unstabel)         GS(Gross wei)      deviceID                          relay state    digit(unit e.g. kg/t)
  ST(stable))          NT(Net wei)               lamp state
  OL(over load)

 lamp state in CAS data format
 | bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 |
    1    stabel    1    hold  print    NET   tare   zero

 relay state in CAS data format
| bit7 | bit6 | bit5 | bit4 | bit3 | bit2 | bit1 | bit0 |
  Out8   Out7   Out6   Out5   Out4   Out3   Out2   Out1
*/
int indicator_WTM_500_data(Common_data_t *common_data) {
  uint8_t set_config[6] = { 0x30, 0x31, 0x52, 0x57, 0x0D, 0x0A };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static cas_22byte_format_t read_data = { 0 };

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(10 / portTICK_PERIOD_MS);
  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }
  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.lamp_states, data + 7, 1);
  memcpy(read_data.data, data + 9, 8);
  memcpy(read_data.relay, data + 17, 1);
  memcpy(read_data.unit, data + 18, 2);

  // Todo
  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  /*stable state set display */
  if (*read_data.lamp_states & 0x40) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }
  /*zero state set display */
  if (*read_data.lamp_states & 0x01) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }
  /*tare state set display */
  if (*read_data.lamp_states & 0x02) {
    common_data->event[STATE_TARE_EVENT] = STATE_TARE_EVENT;
  } else {
    common_data->event[STATE_TARE_EVENT] = STATE_NONE;
  }
  if (read_data.data[0] == '-') {
    common_data->event[STATE_SIGN_EVENT] = STATE_SIGN_EVENT;
  } else {
    common_data->event[STATE_SIGN_EVENT] = STATE_NONE;
  }
  if (strncmp(read_data.states, "ST", 2) == 0 || strncmp(read_data.states, "US", 2) == 0 ||
      strncmp(read_data.states, "OL", 2) == 0) {
    common_data->event[STATE_TRASH_CHECK_EVENT] = STATE_TRASH_CHECK_EVENT;
  } else {
    common_data->event[STATE_TRASH_CHECK_EVENT] = STATE_NONE;
  }

#if INDICATOR_DEBUG
  LOG_BUFFER_HEXDUMP(TAG, &read_data, sizeof(read_data), LOG_INFO);
#endif
  free(data);

  return 0;
}

int cas_zero_command() {
  uint8_t set_config[6] = { 0x30, 0x31, 0x4D, 0x5A, 0x0D, 0x0A };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  char buff[2];

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));

  LOGI(TAG, "uart 485 read data = %s", data);

  memset(&buff, 0x00, sizeof(buff));
  memcpy(buff, data + 2, 2);
  if (strncmp(buff, "MZ", 2) == 0) {
    LOGI(TAG, "zero set success!!");
    free(data);
    return 0;
  } else {
    LOGI(TAG, "zero set fail!!");
    free(data);
    return -1;
  }
}

int cas_tare_command(char *s_data) {
  uint8_t set_config[6] = { 0x30, 0x31, 0x4D, 0x54, 0x0D, 0x0A };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  char buff[2];

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));

  LOGI(TAG, "uart 485 read data = %s", data);

  memset(&buff, 0x00, sizeof(buff));
  memcpy(buff, data + 2, 2);
  if (strncmp(buff, "MT", 2) == 0) {
    memcpy(s_data, data + 9, 8);
    LOGI(TAG, "tare set success!!");
    free(data);
    return 0;
  } else {
    LOGI(TAG, "tare set fail!!");
    free(data);
    return -1;
  }
}
/*
read data format structure of baykon bx11
--------------------------------------------------------------------------------------------------
    |      status     |         Indicated          |             Tare           |                |
STX | STA | STB | STC | D5 | D4 | D3 | D2 | D1 |D0 | D5 | D4 | D3 | D2 | D1 |D0 | CR | CF | CHK  |

Definition Table for Status A (STA)
Bits 0,1 and 2                         | Bits 3,4                | Bit5 | Bit6  | Bit 7          |
0 | 1 | 2      | Decimal Point         | 3 | 4 | Increment size  |              |                |
0 | 0 | 0      | XXXXOO                | 1 | 0 | X1              |              |                |
1 | 0 | 0      | XXXXXO                | 0 | 1 | X2              |              |                |
0 | 1 | 0      | XXXXXX                | 1 | 1 | X5              |              |                |
1 | 1 | 0      | XXXXX.X               |   |   |                 |              |                |
0 | 0 | 1      | XXXX.XX               |   |   |                 | Always 1     |  X             |
1 | 0 | 1      | XXX.XXX               |   |   |                 |              |                |
0 | 1 | 1      | XX.XXXX               |   |   |                 |              |                |
1 | 1 | 1      | X.XXXXX               |   |   |                 |              |                |

Definition Table Status B (STB)
Bit 0                |  0 = Gross               | 1 = Net                       |                |
Bit 1                |  0 = Weight positive     | 1 = Weight negative           |                |
Bit 2                |  0 = No Error            | 1 = Error                     |                |
Bit 3                |  0 = Stable              | 1 = Unstable                  |                |
Bit 4                |  Always 1                |                               |                |
Bit 5                |  Always 1                |                               |                |
Bit 6                |  0 = Not power on zeroed | 1 = Zeroed whit power on zero | weight backup  |
Bit 7                |  X                                                       |                |

Definition Table Status C (STC)
Bit 0                |  Always 0                                                |                |
Bit 1                |  Always 0                                                |                |
Bit 2                |  Always 0                                                |                |
Bit 3                |  Always 0                                                |                |
Bit 4                |  Always 1                                                |                |
Bit 5                |  Always 1                                                |                |
Bit 6                |  Always 0                                                |                |
Bit 7                |  X                                                       |                |
----------------------------------------------------------------------------------------------------
*/
int indicator_BX11_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  Baykon_BX11_data_t read_data = { 0 };
  uint8_t decimal_point = 0;
  uint8_t increment_size = 0;
  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

#if INDICATOR_DEBUG
  LOGE(TAG, "len = %d\n", len);
  LOGI(TAG, "uart 485 read data = %s", data);
  LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);
#endif
  memset(&read_data, 0x00, sizeof(read_data));
  memcpy(read_data.state_A_B_C, data + 1, 3);
  memcpy(read_data.indicator, data + 4, 6);
  memcpy(read_data.tare, data + 10, 6);

  // state check of STA
  decimal_point = read_data.state_A_B_C[0] & 0x07;
  increment_size = read_data.state_A_B_C[0] & 0x18;

  switch (decimal_point) {
    case 0: common_data->DP = DP_100; break;
    case 1: common_data->DP = DP_10; break;
    case 2: common_data->DP = DP_1; break;
    case 3: common_data->DP = DP_0_1; break;
    case 4: common_data->DP = DP_0_01; break;
    case 5: common_data->DP = DP_0_001; break;
    case 6: common_data->DP = DP_0_0001; break;
    case 7: common_data->DP = DP_0_00001; break;
    default: break;
  }
  switch (increment_size) {
    case 8: common_data->IS = IS_X1; break;
    case 16: common_data->IS = IS_X2; break;
    case 24: common_data->IS = IS_X5; break;
    default: break;
  }

  // state check of STB
  if (read_data.state_A_B_C[1] & 0x01)
    common_data->event[STATE_TARE_EVENT] = STATE_TARE_EVENT;
  else
    common_data->event[STATE_TARE_EVENT] = STATE_NONE;

  if (read_data.state_A_B_C[1] & 0x02)
    common_data->event[STATE_SIGN_EVENT] = STATE_SIGN_EVENT;
  else
    common_data->event[STATE_SIGN_EVENT] = STATE_NONE;

  if (read_data.state_A_B_C[1] & 0x08)
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  else
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;

  if (common_data->event[STATE_STABLE_EVENT] == STATE_STABLE_EVENT && atoi(read_data.indicator) == 0)
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  else
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;

  memcpy(common_data->weight_data, read_data.indicator, 6);

  common_data->event[STATE_TRASH_CHECK_EVENT] = STATE_TRASH_CHECK_EVENT;

#if INDICATOR_DEBUG
  LOG_BUFFER_HEXDUMP(TAG, &read_data, sizeof(read_data), LOG_INFO);
#endif

  free(data);
  return 0;
}

int baykon_bx11_zero_command() {
  uint8_t set_config[3] = { 0x5A, 0x0D, 0x0A };
  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  LOGE(TAG, "bx11 zero point command");
  return 0;
}

int baykon_bx11_tare_command(char *s_data) {
  uint8_t set_config[6] = { 0x30, 0x31, 0x4D, 0x54, 0x0D, 0x0A };
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  char buff[2];

  uart_write_data(UART_PORT_NUM, set_config, sizeof(set_config));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));

  LOGI(TAG, "uart 485 read data = %s", data);

  memset(&buff, 0x00, sizeof(buff));
  memcpy(buff, data + 2, 2);
  if (strncmp(buff, "MT", 2) == 0) {
    memcpy(s_data, data + 9, 8);
    LOGI(TAG, "tare set success!!");
    free(data);
    return 0;
  } else {
    LOGI(TAG, "tare set fail!!");
    free(data);
    return -1;
  }
}
