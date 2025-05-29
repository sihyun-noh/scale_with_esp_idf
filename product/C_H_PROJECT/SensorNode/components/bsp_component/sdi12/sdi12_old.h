
#ifndef SDI12_H
#define SDI12_H

#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SDI12_BAUD_RATE 1200
#define SDI12_DATA_BITS UART_DATA_7_BITS  // SDI-12는 7비트 데이터
#define SDI12_PARITY UART_PARITY_EVEN     // 짝수 패리티
#define SDI12_STOP_BITS UART_STOP_BITS_1

#define SDI12_RX_BUF_SIZE 256
#define SDI12_TX_BUF_SIZE 256

#define MAX_RESPONSE_SIZE 75

typedef struct {
  char sensor_address;
  int sdi12_version;     // 예: 13 → 1.3
  char vendor_id[9];     // 공백 포함 8자 + null
  char model[7];         // 6자 + null
  float sensor_version;  // 예: 114 → 1.14
  char serial[14];       // 최대 13자 + null
} SDI12_IdInfo;

/**
 * @brief SDI-12에서 측정 요청(aM!) 등의 응답 형식(atttn) 정보를 저장
 */
typedef struct {
  char *name;
  char Address;       // 센서 주소 (예: '0')
  uint16_t Time;      // 측정까지 걸리는 시간 (ttt)
  uint8_t NumValues;  // 측정 결과 값 개수 (n)
} SDI12_Measure_TypeDef;

/**
 * @brief SDI-12 초기화
 *
 * @param uart_num 사용할 UART 포트 (예: UART_NUM_2)
 * @param tx_pin   SDI-12 통신 TX 핀 번호
 * @param rx_pin   SDI-12 통신 RX 핀 번호
 * @param oe_pin   3상 버퍼 OE 핀 번호 (1=Enable, 0=Hi-Z)
 * @return esp_err_t ESP_OK이면 성공, 그 외 에러
 */
esp_err_t SDI12_Init(uart_port_t uart_num);

/**
 * @brief 센서가 활성화되어 있는지 확인 (a!)
 *
 * @param addr 센서 주소 ('0' ~ '9')
 * @return esp_err_t
 */
esp_err_t SDI12_AckActive(const char addr);

/**
 * @brief 버스에 연결된 모든 센서 주소 검색
 *
 * @param devices 센서 주소를 저장할 버퍼(최소 10바이트)
 */
void SDI12_DevicesOnBus(char *const devices);

/**
 * @brief 센서 ID 요청 (aI!)
 *
 * @param addr 센서 주소
 * @param response 응답 저장 버퍼
 * @param response_len 버퍼 길이
 * @return esp_err_t
 */
// esp_err_t SDI12_GetId(const char addr, char *response, uint8_t response_len);

esp_err_t SDI12_GetId(const char addr, SDI12_IdInfo *info);
/**
 * @brief 센서 주소 변경 (aAb!)
 *
 * @param from_addr 현재 주소
 * @param to_addr   변경할 주소
 * @return esp_err_t
 */
esp_err_t SDI12_ChangeAddr(char *from_addr, char *to_addr);

/**
 * @brief 측정 시작 (aM!)
 *
 * @param addr 센서 주소
 * @param measure_info aM! 응답( atttn ) 파싱 결과 저장
 * @return esp_err_t
 */
esp_err_t SDI12_StartMeasurement(const char addr, SDI12_Measure_TypeDef *measure_info);

/**
 * @brief 센서에서 측정값 가져오기 (aD0! 등)
 *
 * @param addr 센서 주소
 * @param measurement_info StartMeasurement 결과
 * @param data 실제 측정 데이터 저장할 버퍼
 * @return esp_err_t
 */
esp_err_t SDI12_SendData(const char addr, const SDI12_Measure_TypeDef *measurement_info, char *data);

/**
 * @brief 검증 명령 (aV!)
 */
esp_err_t SDI12_StartVerification(const char addr, SDI12_Measure_TypeDef *verification_info);

/**
 * @brief SDI-12 1.3 이상 CRC 체크
 */
uint16_t SDI12_CheckCRC(char *response);

/**
 * @brief CRC 포함 측정 시작 (aMC!)
 */
esp_err_t SDI12_StartMeasurementCRC(const char addr, SDI12_Measure_TypeDef *measurement_info);

#ifdef __cplusplus
}
#endif
#endif  // SDI12_H
