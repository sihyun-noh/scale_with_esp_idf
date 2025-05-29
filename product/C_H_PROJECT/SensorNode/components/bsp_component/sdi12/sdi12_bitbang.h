
// sdi12_bitband.h
#ifndef SDI12_BITBANG_H
#define SDI12_BITBANG_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TICKS_64_US

#ifndef SDI12_IGNORE_PARITY
/**
 * @brief Check the value of the parity bit on reception
 */
// #define SDI12_CHECK_PARITY
#endif

/**
 * @brief   SDI-12 드라이버 RX/TX 버퍼 크기
 */
#define SDI12_BUFFER_SIZE 81

/**
 * @brief   수신 완료를 알리는 이벤트 그룹과 비트
 */
extern EventGroupHandle_t sdi12_event_group;
#define SDI12_EVENT_RX_CHAR (1 << 0)

// enum { LOW = 0, HIGH = 1 };

typedef enum { INPUT = 0, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN } pin_mode_t;

/**
 * @brief   SDI-12 상태 열거형
 */
typedef enum {
  SDI12_DISABLED = 0,  ///< 비활성화 상태
  SDI12_ENABLED,       ///< 활성화 상태
  SDI12_HOLDING,       ///< 버스 점유(LOW) 상태
  SDI12_TRANSMITTING,  ///< 전송 중 상태
  SDI12_LISTENING      ///< 수신 대기 상태
} SDI12_State;

/**
 * @brief   드라이버 초기화
 * @param   rx_pin    SDI-12 버스 RX용 GPIO 번호
 * @param   tx_pin    SDI-12 버스 TX용 GPIO 번호
 * @param   tx_oe     TX 출력 활성화용 GPIO 번호
 */
void sdi12_init(int rx_pin, int tx_pin, int tx_oe);

/**
 * @brief   드라이버 종료 (ISR 제거, 이벤트그룹 삭제 등)
 */
void sdi12_deinit(void);

/**
 * @brief   SDI-12 통신 시작 (마스터 모드)
 */
void sdi12_begin(void);

/**
 * @brief   SDI-12 통신 종료
 */
void sdi12_end(void);

/**
 * @brief   RX 버퍼에 남은 문자 개수 조회
 * @retval  >=0 버퍼에 남은 개수
 * @retval  -1  버퍼 오버플로우 발생
 */
int sdi12_available(void);

/**
 * @brief   버퍼에서 다음 문자 미리보기 (읽어내지 않음)
 * @retval  >=0 읽은 문자
 * @retval  -1  버퍼 비어있음
 */
int sdi12_peek(void);

/**
 * @brief   버퍼에서 다음 문자 읽기
 * @retval  >=0 읽은 문자
 * @retval  -1  버퍼 비어있음
 */
int sdi12_read(void);

/**
 * @brief   RX 버퍼 및 오버플로우 플래그 초기화
 */
void sdi12_clear_buffer(void);

/**
 * @brief   마스터 모드에서 SDI-12 명령 전송
 * @param   cmd             전송할 문자열 (NULL 종단)
 * @param   extraWakeTime   브레이크 이후 추가 웨이크 대기 시간(ms)
 */
void sdi12_send_command(const char *cmd, int extraWakeTime);

/**
 * @brief   슬레이브 모드에서 레코더로 응답 전송
 * @param   resp    응답 문자열 (NULL 종단)
 */
void sdi12_send_response(const char *resp);

/**
 * @brief   타임아웃 시간 설정 (ms)
 */
void sdi12_set_timeout_ms(int to_ms);

/**
 * @brief   타임아웃 시 반환값 설정
 */
void sdi12_set_timeout_value(int16_t to_value);

#ifdef __cplusplus
}
#endif

#endif  // SDI12_BIT_BANG_H
