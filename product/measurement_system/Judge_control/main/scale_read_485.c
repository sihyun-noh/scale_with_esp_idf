/*
  Apply to CAS WTM-500 Indicator
  Apply "3" to setting mode F2-6(data format mode)
  Apply "4" to setting mode F2-7(RS485 - Output mode)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "event_ids.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "log.h"
#include "config.h"
#include "scale_read_485.h"
#include "uart_api.h"
#include "sysevent.h"
#include "esp32/uart_pal.h"
#include "driver/uart.h"

// #define BUF_SIZE 23
#define BUF_SIZE 255
#define INDICATOR_DEBUG 0

#define RD_BUF_SIZE (BUF_SIZE)

typedef enum {
  TASK_OK = 0,
  TASK_ERROR = -1,
} winsen_ze03_err_code;

static const char *TAG = "scale_read_485_task";

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

// DLP-50 target print mode
// format
// ======== WEIGHT ========
// WELCOME TO CAS
// DATE. 2024/08/31
// TIME. 14:02:47
//
//  Weight :       0.10kg
//
// Max 60/150kg, Min 400g, e=d=20/50g
// 20 3d 3d 3d 3d 3d 3d 3d 3d 20 57 45 49 47 48 54
// 20 3d 3d 3d 3d 3d 3d 3d 3d 0a 20 57 45 4c 43 4f
// 4d 45 20 54 4f 20 43 41 53 20 20 20 20 20 20 20
// 20 20 20 0a 20 44 41 54 45 2e 20 32 30 32 34 2f
// 30 38 2f 33 31 0a 20 54 49 4d 45 2e 20 31 33 3a
// 35 36 3a 33 39 0a 0a 20 20 57 65 69 67 68 74 20
// 3a 20 20 20 20 20 20 20 30 2e 30 38 6b 67 0a 0a
// 0a
int indicator_cas_db_2_data(Common_data_t *common_data) {
  int res = 0;
  int read_buf = 120;
  uint8_t *dtmp = (uint8_t *)malloc(read_buf);
  if (dtmp == NULL) {
    LOGI(TAG, "Malloc failed");
    return -1;
  }
  memset(dtmp, 0x00, read_buf);
  res = sysevent_get(SYSEVENT_BASE, WEIGHT_DATA_EVENT, dtmp, read_buf);
  if (res == 0) {
    LOGI(TAG, "weight data : %s", dtmp);
    for (int a = 0; a < read_buf; a++) {
      LOGI(TAG, "data_%d_%c", a, dtmp[a]);
    }
    memcpy(common_data->weight_data, dtmp + 97, 6);
    common_data->spec.unit = UNIT_KG;

    if (strncmp((char *)dtmp + 10, "W", 1) == 0) {
      common_data->check = DATA_OK;
    } else {
      common_data->check = DATA_ERROR;
    }

    free(dtmp);
    return 0;
  }
  free(dtmp);
  return -1;
}

// DEP-50 target print mode
// format
// ===========WEIGHT==========
// Weight :    50.05 kg
// Max 60/150kg, Min 400g, e=d=20/50g
// 20 3d 3d 3d 3d 3d 3d 3d 3d 20 57 45 49 47 48 54
// 20 3d 3d 3d 3d 3d 3d 3d 3d 0a 0a 20 20 57 65 69
// 67 68 74 20 3a 20 20 20 20 20 20 20 32 2e 32 38
// 6b 67 0a 0a 0a 0a 0a 0a 0a 0a 0a 0a 0a
int indicator_cas_db_1_1h_data(Common_data_t *common_data) {
  int res = 0;
  int read_buf = 60;
  uint8_t *dtmp = (uint8_t *)malloc(read_buf);
  if (dtmp == NULL) {
    LOGI(TAG, "Malloc failed");
    return -1;
  }
  memset(dtmp, 0x00, read_buf);
  res = sysevent_get(SYSEVENT_BASE, WEIGHT_DATA_EVENT, dtmp, read_buf);
  if (res == 0) {
    LOGI(TAG, "weight data : %s", dtmp);
    for (int a = 0; a < read_buf; a++) {
      LOGI(TAG, "data_%d_%c", a, dtmp[a]);
    }
    memcpy(common_data->weight_data, dtmp + 44, 6);
    common_data->spec.unit = UNIT_KG;

    if (strncmp((char *)dtmp + 10, "W", 1) == 0) {
      common_data->check = DATA_OK;
    } else {
      common_data->check = DATA_ERROR;
    }

    free(dtmp);
    return 0;
  }
  free(dtmp);
  return -1;
}
// MW2-H
// "설정" 버튼 3초간 -> unit 표시
// "변환" 버튼 -> print 표시
// "설정" 버튼 -> p.print key 설정
// "설정" 버튼 -> 현재 print 키 설정 -> auto-1 로 지정 -> "변환"으로 변경
// "영점" 버튼 -> auto -1, 또는auto-2?? 설정 후 빠져 나오기
// 22:02:46 W:    86.1 g
// 통신선 연결 방식은 2-2,3-3,5-5 형식으로 해야함
// 저울에서 zero가 표시된 후 무게값을 프린트 함, zero가 안뜬 상태에서 무게를 올리면 프린트 안됨.
//
// I (1970-01-01 00:00:07) scale_read_485_task: data_0_
// I (1970-01-01 00:00:07) scale_read_485_task: data_1_2
// I (1970-01-01 00:00:07) scale_read_485_task: data_2_1
// I (1970-01-01 00:00:07) scale_read_485_task: data_3_:
// I (1970-01-01 00:00:07) scale_read_485_task: data_4_4
// I (1970-01-01 00:00:07) scale_read_485_task: data_5_9
// I (1970-01-01 00:00:07) scale_read_485_task: data_6_:
// I (1970-01-01 00:00:07) scale_read_485_task: data_7_5
// I (1970-01-01 00:00:07) scale_read_485_task: data_8_6
// I (1970-01-01 00:00:07) scale_read_485_task: data_9_
// I (1970-01-01 00:00:07) scale_read_485_task: data_10_W
// I (1970-01-01 00:00:07) scale_read_485_task: data_11_:
// I (1970-01-01 00:00:07) scGGale_read_485_task: data_12_
// I (1970-01-01 00:00:07) scale_read_485_task: data_13_
// I (1970-01-01 00:00:07) scale_read_485_task: data_14_
// I (1970-01-01 00:00:07) scale_read_485_task: data_15_
// I (1970-01-01 00:00:07) scale_read_485_task: data_16_8
// I (1970-01-01 00:00:07) scale_read_485_task: data_17_0
// I (1970-01-01 00:00:07) scale_read_485_task: data_18_.
// I (1970-01-01 00:00:07) scale_read_485_task: data_19_5
// I (1970-01-01 00:00:07) scale_read_485_task: data_20_
// I (1970-01-01 00:00:07) scale_read_485_task: data_21_g
//
int indicator_CAS_MW2_H_data(Common_data_t *common_data) {
  int res = 0;
  uint8_t *dtmp = (uint8_t *)malloc(RD_BUF_SIZE);
  if (dtmp == NULL) {
    LOGI(TAG, "Malloc failed");
    return -1;
  }
  memset(dtmp, 0x00, RD_BUF_SIZE);
  res = sysevent_get(SYSEVENT_BASE, WEIGHT_DATA_EVENT, dtmp, RD_BUF_SIZE);
  if (res == 0) {
    LOGI(TAG, "weight data : %s", dtmp);
    for (int a = 0; a < 30; a++) {
      LOGI(TAG, "data_%d_%c", a, dtmp[a]);
    }
    memcpy(common_data->weight_data, dtmp + 12, 8);
    common_data->spec.unit = UNIT_G;

    if (strncmp((char *)dtmp + 10, "W", 1) == 0) {
      common_data->check = DATA_OK;
    } else {
      common_data->check = DATA_ERROR;
    }

    free(dtmp);
    return 0;
  }
  free(dtmp);
  return -1;
}

// ST,GS,00,-    78   g
// I (1970-01-01 00:01:39) scale_read_485_task: read data 0_S
// I (1970-01-01 00:01:39) scale_read_485_task: read data 1_T
// I (1970-01-01 00:01:39) scale_read_485_task: read data 2_,
// I (1970-01-01 00:01:39) scale_read_485_task: read data 3_G
// I (1970-01-01 00:01:39) scale_read_485_task: read data 4_S
// I (1970-01-01 00:01:39) scale_read_485_task: read data 5_,
// I (1970-01-01 00:01:39) scale_read_485_task: read data 6_0
// I (1970-01-01 00:01:39) scale_read_485_task: read data 7_0
// I (1970-01-01 00:01:39) scale_read_485_task: read data 8_,
// I (1970-01-01 00:01:39) scale_read_485_task: read data 9_-
// I (1970-01-01 00:01:39) scale_read_485_task: read data 10_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 11_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 12_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 13_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 14_2
// I (1970-01-01 00:01:40) scale_read_485_task: read data 15_0
// I (1970-01-01 00:01:40) scale_read_485_task: read data 16_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 17_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 18_
// I (1970-01-01 00:01:40) scale_read_485_task: read data 19_g

int indicator_INNOTEM_T25_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static empty_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

  // for (int i = 0; i < 20; i++) {
  //   LOGI(TAG, "read data %d_%c", i, data[i]);
  // }

  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.sign, data + 9, 1);
  memcpy(read_data.data, data + 10, 7);
  memcpy(read_data.unit, data + 18, 2);

  // Todo
  // indicator Min, Max, digit set = g unit
  common_data->spec.scale_Max = 20000;
  common_data->spec.scale_Min = 40;
  common_data->spec.e_d = 2;

  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  // Compare 2digit
  if (strncmp(read_data.unit, " g", 2) == 0) {
    // copy to weight data
    weight = (float)(atoi(read_data.data) * 0.001);
    // LOGI(TAG, "g = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_G;

  } else if (strncmp(read_data.unit, "kg", 2) == 0) {
    sscanf(read_data.data, "%lf", &weight);
    // LOGI(TAG, "kg = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_KG;

  } else {
    return 0;
  }

  /*stable state set display */
  if (strncmp(read_data.states, "ST", 2) == 0) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }

  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  if (read_data.sign[0] == '-') {
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
/**
 *
 * * MAX: 150kg, Min 500g e=d 10g
 * RS-232 전송 방법
 * P Count2
 */
// format : cas 22 byte
int indicator_CAS_hb_hbi_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static cas_22byte_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }
  //
  // LOGE(TAG, "len = %d\n", len);
  // LOGI(TAG, "uart 485 read data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);
  //
  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.lamp_states, data + 7, 1);
  memcpy(read_data.data, data + 9, 8);
  memcpy(read_data.unit, data + 18, 2);

  // Todo
  // indicator Min, Max, digit set = g unit
  common_data->spec.scale_Max = 30000;  // 30kg
  common_data->spec.scale_Min = 200;
  common_data->spec.e_d = 10;

  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  if (strncmp(read_data.unit, "kg", 2) == 0) {
    common_data->spec.unit = UNIT_KG;
  }
  // check weight data
  sscanf(read_data.data, "%lf", &weight);
  // LOGI(TAG, "zero check weight = %lf", weight);

  /*stable state set display */
  if (*read_data.lamp_states & 0x40) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }
  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
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

/**
 *
 * 저울의 기본 프로토콜 = baudrate : 9600bpc, Data bit : 8, stop bit: 1, Parity : none
 * "영점" 누른 상태에서 전원 -> 영점 버튼 한번더 누르면 통신 set mode
 * "모드" 버튼을 누르면 rs-232 설정값 변경 -> rS-Co 모드로(데이터 연속적으로)
 * MAX: 30kg, Min 200g e=d 10g
 */
// format : cas 22 byte

int indicator_CAS_sw_11_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static cas_22byte_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }
  //
  // LOGE(TAG, "len = %d\n", len);
  // LOGI(TAG, "uart 485 read data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);
  //
  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.lamp_states, data + 7, 1);
  memcpy(read_data.data, data + 9, 8);
  memcpy(read_data.unit, data + 18, 2);

  // Todo
  // indicator Min, Max, digit set = g unit
  // common_data->spec.scale_Max = 30000;  // 30kg
  // common_data->spec.scale_Min = 200;
  // common_data->spec.e_d = 10;

  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  if (strncmp(read_data.unit, "kg", 2) == 0) {
    common_data->spec.unit = UNIT_KG;
    sscanf(read_data.data, "%lf", &weight);

  } else if (strncmp(read_data.unit, "g ", 2) == 0) {
    common_data->spec.unit = UNIT_G;
    weight = (float)(atoi(read_data.data) * 0.001);
  } else {
    LOGI(TAG, "invalid data");
    return 0;
  }
  // LOGI(TAG, "zero check weight = %lf", weight);

  /*stable state set display */
  if (*read_data.lamp_states & 0x40) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }
  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
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

/**
 * 저울의 기본 프로토콜 = baudrate : 9600bpc, Data bit : 8, stop bit: 1, Parity : none
 * "*" 누른 상태에서 전원 -> UP -
 * "*" 키를 3회 누릅니다 3   11(default)
 * "영점" 자리수 이동
 * "용기" ㅅ숫자 설정
 * "*" 키 4회 누르면 설정값이 저장되고 계량모드화면으로 이동
 * pw-200 은 base unit : g(Display unit) 1kg == 1000
 * MAX: 6kg, Min 40g
 */
// S : stable
// U : Unstable
// sign : - , +
// format : _|_|S|+|_|_|_|_|_|_| |g|CRLF

int indicator_ACOM_pw_200_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static empty_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

  // LOGE(TAG, "len = %d\n", len);
  // LOGI(TAG, "uart 485 read data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);
  //
  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data + 2, 1);
  memcpy(read_data.sign, data + 3, 1);
  memcpy(read_data.data, data + 4, 6);
  memcpy(read_data.unit, data + 11, 1);
  //  Todo

  // indicator Min, Max, digit set = g unit
  common_data->spec.scale_Max = 30000;
  common_data->spec.scale_Min = 10;
  common_data->spec.e_d = 2;

  // check weight data & unit
  if (read_data.unit[0] == 'g') {
    // copy to weight data
    //    LOGI(TAG, "weight read data_g = %s", read_data.data);
    weight = (float)(atoi(read_data.data) * 0.001);
    memcpy(common_data->weight_data, read_data.data, 6);
    common_data->spec.unit = UNIT_G;
  } else {
    // LOGI(TAG, "weight read data_other = %s", read_data.data);
    sscanf(read_data.data, "%lf", &weight);
    memcpy(common_data->weight_data, read_data.data, 6);
  }

  /*stable state set display */
  if (read_data.states[0] == 'S') {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }

  /*zero state set display */
  if (read_data.states[0] == 'S' && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  if (read_data.sign[0] == '-') {
    common_data->event[STATE_SIGN_EVENT] = STATE_SIGN_EVENT;
  } else {
    common_data->event[STATE_SIGN_EVENT] = STATE_NONE;
  }

  if (read_data.states[0] == 'S' || read_data.states[0] == 'U') {
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

/**
 * "샘플"키 누르고 있음 들어가짐
 * Func 항목에서 프린트 버튼 누르면 설정 화면으로 바뀜
 * 시리얼 데이터를 사용하기 위한 설정은
 * Prt 0  --> 0
 * BPS : 9600  --> 2
 * btPr : 8비트  --> 2
 */
// ST,+00000.00  gCRLF

int indicator_AND_CB_12K_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static empty_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

  // LOGE(TAG, "len = %d\n", len);
  // LOGI(TAG, "uart 485 read data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);

  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.sign, data + 3, 1);
  memcpy(read_data.data, data + 4, 8);
  memcpy(read_data.unit, data + 13, 2);
  //  Todo
  // indicator Min, Max, digit set = g unit
  common_data->spec.scale_Max = 12000;
  common_data->spec.scale_Min = 50;
  common_data->spec.e_d = 1;

  // check weight data & unit
  // Compare 2digit
  if (strncmp(read_data.unit, " g", 2) == 0) {
    // copy to weight data
    // weight = (float)(atoi(read_data.data) * 0.001);
    sscanf(read_data.data, "%lf", &weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_G;
  } else {
    sscanf(read_data.data, "%lf", &weight);
    memcpy(common_data->weight_data, read_data.data, 8);
  }

  /*stable state set display */
  if (strncmp(read_data.states, "ST", 2) == 0) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }

  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  if (read_data.sign[0] == '-') {
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
// 설정 키
// 비번 101010
// 입력버튼으로 "trn" 나올때까지
// 설정 번호 "2" 연속데이터
// 데이터 포멧이  저울이 안정일때 NET: (대문자), 안정이지 않을 때 net:(소문자)
// I (1970-01-01 00:06:20) scale_read_485_task: read data 12_N
// I (1970-01-01 00:06:20) scale_read_485_task: read data 13_E
// I (1970-01-01 00:06:20) scale_read_485_task: read data 14_T
// I (1970-01-01 00:06:20) scale_read_485_task: read data 15_:
// I (1970-01-01 00:06:20) scale_read_485_task: read data 16_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 17_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 18_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 19_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 20_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 21_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 22_2
// I (1970-01-01 00:06:20) scale_read_485_task: read data 23_5
// I (1970-01-01 00:06:20) scale_read_485_task: read data 24_2
// I (1970-01-01 00:06:20) scale_read_485_task: read data 25_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 26_
// I (1970-01-01 00:06:20) scale_read_485_task: read data 27_g

int indicator_EC_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static empty_format_t read_data = { 0 };
  double weight = 0;
  int length = 0;
  // uart_flush(UART_PORT_NUM);
  // vTaskDelay(100 / portTICK_PERIOD_MS);

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }
  // LOGE(TAG, "read data len %d", len);
  // for (int i = 0; i < 100; i++) {
  //   LOGI(TAG, "read data %d_%c", i, data[i]);
  // }

  // 문자열 탐색 strstr()로 서브문자를 찾은 후 첫번재 인자를 받는다.
  char *stable_position = strstr((const char *)data, "NET:");
  char *unstable_position = strstr((const char *)data, "net:");
  char *net_data_position = NULL;
  memset(&read_data, 0x00, sizeof(read_data));
  if (stable_position != NULL) {
    net_data_position = stable_position + 6;
    memcpy(read_data.states, "ST", 2);
    memcpy(read_data.sign, stable_position + 5, 1);
    memcpy(read_data.unit, stable_position + 15, 2);
  }
  if (unstable_position != NULL) {
    net_data_position = unstable_position + 6;
    memcpy(read_data.states, "US", 2);
    memcpy(read_data.sign, unstable_position + 5, 1);
    memcpy(read_data.unit, unstable_position + 15, 2);
  }

  // copy to weight data
  memcpy(read_data.data, net_data_position, 7);

  // LOGE(TAG, "read data %s", read_data.data);
  // LOGE(TAG, "read data state %s", read_data.states);
  // LOGE(TAG, "read data unit %s", read_data.unit);
  // LOGE(TAG, "read data sign %s", read_data.sign);
  // Compare 2digit
  if (strncmp(read_data.unit, "g", 1) == 0) {
    // copy to weight data
    weight = (float)(atoi(read_data.data) * 0.001);
    // LOGI(TAG, "g = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_G;

  } else if (strncmp(read_data.unit, "kg", 2) == 0) {
    sscanf(read_data.data, "%lf", &weight);
    // LOGI(TAG, "kg = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_KG;

  } else {
    return -1;
  }

  /*stable state set display */
  if (strncmp(read_data.states, "ST", 2) == 0) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }

  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  if (read_data.sign[0] == '-') {
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
// Hold the sample button for 2 to 3 seconds to enter setting mode
// P cont mode
// 1  | 2  | 3  |  4  |  5  |  6  | 7 ~ 14  |  15 ~ 18  | 19  |  20  |
// ST,GS,+ 0.879 g
// I (1970-01-01 00:01:18) scale_read_485_task: read data 0_S
// I (1970-01-01 00:01:18) scale_read_485_task: read data 1_T
// I (1970-01-01 00:01:18) scale_read_485_task: read data 2_,
// I (1970-01-01 00:01:18) scale_read_485_task: read data 3_G
// I (1970-01-01 00:01:18) scale_read_485_task: read data 4_S
// I (1970-01-01 00:01:18) scale_read_485_task: read data 5_,
// I (1970-01-01 00:01:18) scale_read_485_task: read data 6_+
// I (1970-01-01 00:01:18) scale_read_485_task: read data 7_
// I (1970-01-01 00:01:18) scale_read_485_task: read data 8_0
// I (1970-01-01 00:01:18) scale_read_485_task: read data 9_.
// I (1970-01-01 00:01:18) scale_read_485_task: read data 10_9
// I (1970-01-01 00:01:18) scale_read_485_task: read data 11_3
// I (1970-01-01 00:01:18) scale_read_485_task: read data 12_4
// I (1970-01-01 00:01:18) scale_read_485_task: read data 13_5
// I (1970-01-01 00:01:18) scale_read_485_task: read data 14_
// I (1970-01-01 00:01:18) scale_read_485_task: read data 15_k
// I (1970-01-01 00:01:18) scale_read_485_task: read data 16_g
// I (1970-01-01 00:01:18) scale_read_485_task: read data 17_
// I (1970-01-01 00:01:18) scale_read_485_task: read data 18_
// I (1970-01-01 00:01:18) scale_read_485_task: read data 19_
//
//
// I (1970-01-01 00:01:58) scale_read_485_task: read data 0_S
// I (1970-01-01 00:01:58) scale_read_485_task: read data 1_T
// I (1970-01-01 00:01:58) scale_read_485_task: read data 2_,
// I (1970-01-01 00:01:58) scale_read_485_task: read data 3_G
// I (1970-01-01 00:01:58) scale_read_485_task: read data 4_S
// I (1970-01-01 00:01:58) scale_read_485_task: read data 5_,
// I (1970-01-01 00:01:58) scale_read_485_task: read data 6_+
// I (1970-01-01 00:01:58) scale_read_485_task: read data 7_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 8_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 9_6
// I (1970-01-01 00:01:58) scale_read_485_task: read data 10_7
// I (1970-01-01 00:01:58) scale_read_485_task: read data 11_9
// I (1970-01-01 00:01:58) scale_read_485_task: read data 12_.
// I (1970-01-01 00:01:58) scale_read_485_task: read data 13_0
// I (1970-01-01 00:01:58) scale_read_485_task: read data 14_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 15_g
// I (1970-01-01 00:01:58) scale_read_485_task: read data 16_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 17_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 18_
// I (1970-01-01 00:01:58) scale_read_485_task: read data 19_

int indicator_EC_D_Serise_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static empty_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

  // for (int i = 0; i < 50; i++) {
  //   LOGI(TAG, "read data %d_%c", i, data[i]);
  // }

  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.sign, data + 6, 1);
  memcpy(read_data.data, data + 8, 6);
  memcpy(read_data.unit, data + 15, 2);

  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  // Compare 2digit
  if (strncmp(read_data.unit, "g ", 2) == 0) {
    // copy to weight data
    weight = (float)(atoi(read_data.data) * 0.001);
    // LOGI(TAG, "g = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_G;

  } else if (strncmp(read_data.unit, "kg", 2) == 0) {
    sscanf(read_data.data, "%lf", &weight);
    // LOGI(TAG, "kg = %f", weight);
    memcpy(common_data->weight_data, read_data.data, 8);
    common_data->spec.unit = UNIT_KG;

  } else {
    return -1;
  }

  /*stable state set display */
  if (strncmp(read_data.states, "ST", 2) == 0) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }

  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  if (read_data.sign[0] == '-') {
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

// US,NT,+000.008kg
int indicator_CAS_NT301A_data(Common_data_t *common_data) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  static cas_22byte_format_t read_data = { 0 };
  double weight = 0;

  int len = uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  vTaskDelay(100 / portTICK_PERIOD_MS);
  if (len <= 0) {
    LOGE(TAG, "Invalid data. or No data ");
    LOGE(TAG, "len = %d\n", len);
    LOGE(TAG, "uart 485 read data = %s", data);
    free(data);
    return -1;
  }

  // LOGE(TAG, "len = %d\n", len);
  // LOGI(TAG, "uart 485 read data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, len, LOG_INFO);
  memset(&read_data, 0x00, sizeof(read_data));

  memcpy(read_data.states, data, 2);
  memcpy(read_data.measurement_states, data + 3, 2);
  memcpy(read_data.lamp_states, data + 7, 1);
  memcpy(read_data.data, data + 9, 8);
  memcpy(read_data.unit, data + 18, 2);

  // Todo
  // copy to weight data
  memcpy(common_data->weight_data, read_data.data, 8);

  // check weight data
  sscanf(read_data.data, "%lf", &weight);
  // LOGI(TAG, "zero check weight = %lf", weight);

  /*stable state set display */
  if (*read_data.lamp_states & 0x40) {
    common_data->event[STATE_STABLE_EVENT] = STATE_STABLE_EVENT;
  } else {
    common_data->event[STATE_STABLE_EVENT] = STATE_NONE;
  }
  /*zero state set display */
  if (strncmp(read_data.states, "ST", 2) == 0 && weight == 0.0) {
    common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  } else {
    common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  }

  // if (*read_data.lamp_states & 0x01) {
  //   common_data->event[STATE_ZERO_EVENT] = STATE_ZERO_EVENT;
  // } else {
  //   common_data->event[STATE_ZERO_EVENT] = STATE_NONE;
  // }

  /*tare state set display */
  // if (*read_data.lamp_states & 0x02) {
  //   common_data->event[STATE_TARE_EVENT] = STATE_TARE_EVENT;
  // } else {
  //   common_data->event[STATE_TARE_EVENT] = STATE_NONE;
  // }

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
  vTaskDelay(100 / portTICK_PERIOD_MS);
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
    case 3: common_data->DP = DP_KG_0_1; break;
    case 4: common_data->DP = DP_KG_0_01; break;
    case 5: common_data->DP = DP_KG_0_001; break;
    case 6: common_data->DP = DP_G_0_1; break;
    case 7: common_data->DP = DP_G_0_01; break;
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

void update_weight(printer_data_t *pdata, const char *new_weight) {
  int new_weight_len = strlen(new_weight);
  int new_size = pdata->size + (new_weight_len - pdata->weight_len);

  unsigned char *new_data = malloc(new_size);

  memcpy(new_data, pdata->data, pdata->weight_pos);
  memcpy(new_data + pdata->weight_pos, new_weight, new_weight_len);
  memcpy(new_data + pdata->weight_pos + new_weight_len, pdata->data + pdata->weight_pos + pdata->weight_len,
         pdata->size - pdata->weight_pos - pdata->weight_len);

  free(pdata->data);

  pdata->data = new_data;
  pdata->size = new_size;
  pdata->weight_len = new_weight_len;
}

printer_data_t *init_data(const unsigned char *initial_data, int len, int weight_position, int weight_length) {
  printer_data_t *pdata = malloc(sizeof(printer_data_t));
  pdata->data = malloc(len);
  memcpy(pdata->data, initial_data, len);
  pdata->size = len;
  pdata->weight_pos = weight_position;
  pdata->weight_len = weight_length;
  return pdata;
}

void print_data(const printer_data_t *pdata) {
  for (int i = 0; i < pdata->size; i++) {
    LOGI(TAG, "%02x ", pdata->data[i]);
  }
}

void weight_print_msg(char *s_weight, weight_unit_t unit) {
  unsigned char print_data[40] = { 0 };
  unsigned char print_data_head[] = { 0x20, 0x20, 0x20, 0x37, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                                      0x31, 0x20, 0x20, 0x20, 0x31, 0x33, 0x37, 0x38, 0x20, 0x20, 0x02 };
  unsigned char print_data_body[] = { 0x33, 0x32, 0x2e, 0x36, 0x30 };  // avable using data format 0.000, 00.00, 000.0
  // 기존 0x02,0x6b,0x67,0x0d,0x0a 인데 6자리나오게 하려고 앞 0x02 자름, 프린트 표시 10.740kg(기존 10.740 k 만 표시됨)
  unsigned char print_data_tail_unit_kg[] = { 0x6b, 0x67, 0x0d, 0x0a };  // " kg\r\n"
  unsigned char print_data_tail_unit_g[] = { 0x20, 0x67, 0x0d, 0x0a };   // "  g\r\n"

  char *weight = s_weight;
  int weight_len = strlen(weight);

  memcpy(print_data, print_data_head, sizeof(print_data_head));
  memcpy(print_data + sizeof(print_data_head), weight, weight_len);
  if (unit == UNIT_KG) {
    memcpy(print_data + sizeof(print_data_head) + weight_len, print_data_tail_unit_kg, sizeof(print_data_tail_unit_kg));
  } else if (unit == UNIT_G) {
    memcpy(print_data + sizeof(print_data_head) + weight_len, print_data_tail_unit_g, sizeof(print_data_tail_unit_g));
  }
  LOGI(TAG, "before data : %s", print_data);

  // printer_data_t *pdata = init_data(print_data_body, sizeof(print_data_body), 26, 2);
  // LOGI(TAG, "before data:");
  // print_data(pdata);
  // LOGI(TAG, "after data:");
  // update_weight(pdata, "10.00");
  // print_data(pdata);
  // free(pdata->data);
  // free(pdata);

  uart_write_data(UART_PORT_NUM, print_data, sizeof(print_data));
  vTaskDelay(100 / portTICK_PERIOD_MS);
}

int cas_dlp_label_weight_print_msg(char *s_weight) {
  uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
  char print_data[40] = { 0 };
  const char *cas_print_effective_Qualification[] = { "SER" };
  size_t num_qualifications = sizeof(cas_print_effective_Qualification) / sizeof(cas_print_effective_Qualification[0]);

  unsigned char print_data_form[] = { 0x46, 0x52, 0x22, 0x46, 0x4f, 0x52, 0x4d, 0x30, 0x22, 0x0a };  // form 3
  unsigned char print_data_head[] = { 0x3f, 0x0a };
  unsigned char print_data_formmat_value[] = { 0x20, 0x20, 0x20, 0x30, 0x2e, 0x33, 0x30, 0x0a };
  unsigned char print_data_body[] = { 0x33, 0x32, 0x2e, 0x36, 0x30 };  // avable using data format 0.000, 00.00, 000.0
  unsigned char print_data_0a[] = { 0x0a };                            // 0x0a

  char *weight = s_weight;
  int weight_len = strlen(weight);

  // memcpy(print_data, print_data_head, sizeof(print_data_head));
  // memcpy(print_data + sizeof(print_data_head), weight, weight_len);
  // memcpy(print_data + sizeof(print_data_head) + weight_len, print_data_tail, sizeof(print_data_tail));
  //
  memcpy(print_data, print_data_form, sizeof(print_data_form));
  memcpy(print_data + strlen(print_data), print_data_head, sizeof(print_data_head));
  memcpy(print_data + strlen(print_data), print_data_formmat_value, sizeof(print_data_formmat_value));
  memcpy(print_data + strlen(print_data), weight, weight_len);
  memcpy(print_data + strlen(print_data), print_data_0a, sizeof(print_data_0a));

  // memcpy(print_data + strlen(print_data), print_data_tail, sizeof(print_data_tail));

  // LOGI(TAG, "CAS DLP_LABEL create data for print: %s", print_data);
  // LOGI(TAG, "CAS DLP_LABEL strlen : %d", strlen(print_data));
  // LOG_BUFFER_HEXDUMP(TAG, print_data, sizeof(print_data), LOG_LOCAL_LEVEL);

  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  // LOGE(TAG, "garbage read data = %s", data);

  memset(data, 0x00, BUF_SIZE);
  uart_hal_flush(UART_PORT_NUM);
  uart_write_data(UART_PORT_NUM, (const unsigned char *)print_data, strlen(print_data));
  vTaskDelay(200 / portTICK_PERIOD_MS);
  uart_read_data(UART_PORT_NUM, data, (BUF_SIZE - 1));
  // LOGE(TAG, "print response data = %s", data);
  // LOG_BUFFER_HEXDUMP(TAG, data, sizeof(data), LOG_LOCAL_LEVEL);

  for (int i = 0; num_qualifications > i; i++) {
    if (strncmp((const char *)data, cas_print_effective_Qualification[i],
                strlen(cas_print_effective_Qualification[i])) == 0) {
      LOGI(TAG, "success data = %s", data);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      free(data);
      return 0;
    }
  }
  free(data);
  return -1;
}

void real_print_cmd() {
  unsigned char print_data_tail[] = { 0x30, 0x30, 0x0a, 0x50, 0x31, 0x2c, 0x31, 0x0a, 0x0a };  // "p1,1
  uart_hal_flush(UART_PORT_NUM);
  uart_write_data(UART_PORT_NUM, (const unsigned char *)print_data_tail, strlen((char *)print_data_tail));
  vTaskDelay(100 / portTICK_PERIOD_MS);
}
