

// sdi12_bitbang.c
#include "sdi12_bitbang.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "hal/gpio_types.h"

static const char *TAG = "SDI12";

// ── 상수 정의 ─────────────────────────
// static const uint32_t BITWIDTH_US = 833;     ///< 1200 baud당 비트 시간 (μs)
static const uint32_t LINEBREAK_US = 12300;  ///< ≥12ms 브레이크 (μs)
static const uint32_t MARKING_US = 8500;     ///< ≥8.33ms 마킹 (μs)

// ── 전역 변수 ─────────────────────────
EventGroupHandle_t sdi12_event_group = NULL;

// ── 내부 RX 버퍼 ────────────────────────
static uint8_t _rxBuf[SDI12_BUFFER_SIZE];
static volatile uint8_t _rxHead, _rxTail = 0;
static volatile bool _bufOverflow = false;

// ── 핀 및 상태 ─────────────────────────
static int _pinRX, _pinTX, _pinTXOE;
static SDI12_State _state = SDI12_DISABLED;

// ── 타임아웃 설정 ───────────────────────
static int32_t _timeout_us = 150000;   ///< 기본 150ms
static int16_t _timeoutValue = -9999;  ///< 타임아웃 리턴값

// ── 비트 뱅잉용 내부 변수 ─────────────────
static uint32_t _prevTimeUs;
static uint8_t _rxState;
static uint8_t _rxMask;
static uint8_t _rxValue;

// ── 내부 함수 프로토타입 ─────────────────
static void IRAM_ATTR sdi12_receive_isr(void *arg);
static uint16_t mul8x8to16(uint8_t x, uint8_t y);
static uint8_t parity_even_bit(uint8_t v);
static void startChar(void);
static void IRAM_ATTR charToBuffer(uint8_t c);
static void setState(SDI12_State st);
static void setPinInterrupts(bool en);
static void wakeSensors(int extraWakeTime);
static void writeChar(uint8_t outChar);

/* clang-format off */
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'),\
  ((byte) & 0x40 ? '1' : '0'),\
  ((byte) & 0x20 ? '1' : '0'),\
  ((byte) & 0x10 ? '1' : '0'),\
  ((byte) & 0x08 ? '1' : '0'),\
  ((byte) & 0x04 ? '1' : '0'),\
  ((byte) & 0x02 ? '1' : '0'),\
  ((byte) & 0x01 ? '1' : '0')
/* clang-format on */

#ifdef TICKS_64_US
typedef uint32_t sdi12timer_t;
#else
typedef uint32_t sdi12timer_t;
#endif
static inline sdi12timer_t IRAM_ATTR READTIME() {
#ifdef TICKS_64_US
  return (sdi12timer_t)(esp_timer_get_time() >> 6);  // 64µs 단위 tick
#else
  return (sdi12timer_t)(esp_timer_get_time());
#endif
}

void pinMode(gpio_num_t pin, pin_mode_t mode) {
  gpio_config_t io_conf = { .pin_bit_mask = (1ULL << pin),
                            .intr_type = GPIO_INTR_DISABLE,
                            .mode = GPIO_MODE_DISABLE,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE };

  switch (mode) {
    case INPUT: io_conf.mode = GPIO_MODE_INPUT; break;
    case OUTPUT: io_conf.mode = GPIO_MODE_OUTPUT; break;
    case INPUT_PULLUP:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
      break;
    case INPUT_PULLDOWN:
      io_conf.mode = GPIO_MODE_INPUT;
      io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
      break;
    default: return;  // 잘못된 모드 처리
  }

  gpio_config(&io_conf);
}
void sdi12_init(int rx_pin, int tx_pin, int tx_oe) {
  static bool isr_service_installed = false;  // ISR 서비스 설치 여부 추적

  _pinRX = rx_pin;
  _pinTX = tx_pin;
  _pinTXOE = tx_oe;

#if 1
  // GPIO 모드 설정
  gpio_config_t io_conf = { .pin_bit_mask = (1ULL << _pinRX) | (1ULL << _pinTX) | (1ULL << _pinTXOE),
                            .mode = GPIO_MODE_INPUT_OUTPUT,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_ANYEDGE };
  gpio_config(&io_conf);

#else
  // 1. RX 핀만 개별 설정 (pull-up 활성화)
  gpio_config_t io_rx = { .pin_bit_mask = (1ULL << _pinRX),
                          .mode = GPIO_MODE_INPUT,
                          .pull_up_en = GPIO_PULLUP_ENABLE,
                          .pull_down_en = GPIO_PULLDOWN_DISABLE,
                          .intr_type = GPIO_INTR_ANYEDGE };
  gpio_config(&io_rx);

  //  2. TX, TXOE 핀 설정 (pull-up 비활성화)
  gpio_config_t io_tx_oe = { .pin_bit_mask = (1ULL << _pinTX) | (1ULL << _pinTXOE),
                             .mode = GPIO_MODE_OUTPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE };
  gpio_config(&io_tx_oe);
#endif

  // ISR 서비스 등록 (한 번만 수행)
  if (!isr_service_installed) {
    esp_err_t err = gpio_install_isr_service(0);
    if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
      isr_service_installed = true;
    } else {
      ESP_LOGE(TAG, "Failed to install ISR service: %s", esp_err_to_name(err));
    }
  }

  // gpio_isr_handler_add(_pinRX, sdi12_receive_isr, NULL);
  //  gpio_isr_handler_add(_pinRX, sdi12_receive_isr, (void *)1);

  // 이벤트 그룹 생성
  if (sdi12_event_group == NULL) {
    sdi12_event_group = xEventGroupCreate();
  }

  ESP_LOGI(TAG, "SDI-12 초기화: RX=%d, TX=%d, OE=%d", rx_pin, tx_pin, tx_oe);
}
void sdi12_deinit(void) {
  gpio_isr_handler_remove(_pinRX);
  vEventGroupDelete(sdi12_event_group);
  sdi12_event_group = NULL;
  ESP_LOGI(TAG, "SDI-12 해제 완료");
}

void sdi12_begin(void) {
  setState(SDI12_HOLDING);
}

void sdi12_end(void) {
  setState(SDI12_DISABLED);
}

int sdi12_available(void) {
  if (_bufOverflow)
    return -1;
  return (_rxTail + SDI12_BUFFER_SIZE - _rxHead) % SDI12_BUFFER_SIZE;
}

int sdi12_peek(void) {
  if (_rxHead == _rxTail)
    return -1;
  return _rxBuf[_rxHead];
}

int sdi12_read(void) {
  if (_rxHead == _rxTail)
    return -1;
  uint8_t nextChar = _rxBuf[_rxHead];
  _rxHead = (_rxHead + 1) % SDI12_BUFFER_SIZE;
  _bufOverflow = false;
  return nextChar;
}

void sdi12_clear_buffer(void) {
  _rxHead = _rxTail = 0;
  _bufOverflow = false;
}

void sdi12_send_command(const char *cmd, int extraWakeTime) {
  // setState(SDI12_TRANSMITTING);
  wakeSensors(extraWakeTime);
  for (int unsigned i = 0; i < strlen(cmd); i++) {
    writeChar((uint8_t)cmd[i]);
  }
  setState(SDI12_LISTENING);
}

void sdi12_send_response(const char *resp) {
  setState(SDI12_TRANSMITTING);
  gpio_set_level((gpio_num_t)_pinRX, 1);  // 마킹 시작
  esp_rom_delay_us(MARKING_US);
  for (int unsigned i = 0; i < strlen(resp); i++) {
    writeChar((uint8_t)resp[i]);
  }
  setState(SDI12_LISTENING);
}

void sdi12_set_timeout_ms(int to_ms) {
  _timeout_us = to_ms * 1000;
}

void sdi12_set_timeout_value(int16_t to_value) {
  _timeoutValue = to_value;
}

// ── 내부 헬퍼 함수 ───────────────────────

static uint16_t mul8x8to16(uint8_t x, uint8_t y) {
  return x * y;
}

static uint8_t parity_even_bit(uint8_t v) {
  uint8_t parity = 0;
  while (v) {
    parity = !parity;
    v = v & (v - 1);
  }
  return parity;
}

// Creates a blank slate of bits for an incoming character
static void startChar(void) {
  _rxState = 0x00;
  _rxMask = 0x01;
  _rxValue = 0x00;
}
static uint8_t prevChar = 0;
static void IRAM_ATTR charToBuffer(uint8_t c) {
  uint8_t next = (_rxTail + 1) % SDI12_BUFFER_SIZE;
  if (next == _rxHead) {
    _bufOverflow = true;
  } else {
    _rxBuf[_rxTail] = c;
    _rxTail = next;
  }
#if 0
  if (prevChar == '\r' && c == '\n') {
    // sdi12_line_ready = true;
    // 이벤트 비트 세팅
    BaseType_t hp = pdFALSE;
    xEventGroupSetBitsFromISR(sdi12_event_group, SDI12_EVENT_RX_CHAR, &hp);
    if (hp)
      portYIELD_FROM_ISR();
  }
#endif
  prevChar = c;
}

#ifdef TICKS_64_US
/**
 * 15625 'ticks'/sec = 64 µs / 'tick'
 * (1 sec/1200 bits) * (1 tick/64 µs) = 13.0208 ticks/bit
 *
 * The 8-bit timer rolls over after 256 ticks, 19.66085 bits, or 16.38505 ms
 * (256 ticks/roll-over) * (1 bit/13.0208 ticks) = 19.66085 bits
 * (256 ticks/roll-over) * (1 sec/15625 ticks) = 16.38505 milliseconds
 */
#define TICKS_PER_BIT 13
/**
 * 1/(13.0208 ticks/bit) * 2^10 = 78.6432
 */
#define BITS_PER_TICK_Q10 79
#define RX_WINDOW_FUDGE   2  // 2, 4,6,8

#else

/**
 * Using `micros()` 1 "tick" is 1 µsec
 * (1 sec/1200 bits) * (1 tick/1 µs) * (1000000 µsec/sec)= 833.33333 ticks/bit
 *
 * The 32-bit timer rolls over after 4294967296 ticks, or 4294.9673 seconds
 */
#define TICKS_PER_BIT 833UL

#define RX_WINDOW_FUDGE 50  // 50
#endif

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

bool _parityFailure;

static void IRAM_ATTR sdi12_receive_isr(void *arg) {
  sdi12timer_t now = READTIME();
  uint8_t level = gpio_get_level((gpio_num_t)_pinRX);

  if (_rxState == 0xFF) {
    // 스타트 비트 대기
    if (level == HIGH) {
      return;
    }
    startChar();
  }

// 데이터 비트 수신 처리
#ifdef TICKS_64_US
  //  uint16_t bitsPassed = (mul8x8to16((uint8_t)(now - _prevTimeUs) + RX_WINDOW_FUDGE, BITS_PER_TICK_Q10)) >> 10;

  uint16_t deltaTicks = (uint16_t)(now - _prevTimeUs);
  uint16_t bitsPassed = (mul8x8to16(deltaTicks + RX_WINDOW_FUDGE, BITS_PER_TICK_Q10)) >> 10;
#else
  uint16_t bitsPassed = (uint16_t)((((now - _prevTimeUs) + RX_WINDOW_FUDGE) / TICKS_PER_BIT));
#endif
  uint8_t bitsLeft = 9 - _rxState;
  bool nextChar = bitsPassed > bitsLeft;
  uint8_t bitsFrame = nextChar ? bitsLeft : bitsPassed;

  _rxState += bitsFrame;

  if (level == LOW) {
    // LOW = 1 (inverse logic)
    while (bitsFrame-- > 0) {
      // for each of the bits that happened in this frame

      _rxValue |= _rxMask;     // Add a 1 to the LSB/right-most place of our character
                               // value from the mask
      _rxMask = _rxMask << 1;  // Shift the 1 in the mask up by one position
    }
    // And shift the 1 in the mask up by one more position for the current bit.
    // It's HIGH/0 now, so we don't use `|=` with the mask for this last one.
    _rxMask = _rxMask << 1;

  } else {
    // HIGH = 0
    //_rxMask <<= bitsFrame;
    _rxMask = _rxMask << (bitsFrame - 1);
    _rxValue |= _rxMask;
  }

  if (_rxState > 7) {
#ifdef SDI12_CHECK_PARITY
    uint8_t rxParity = bitRead(_rxValue, 7);  // pull out the parity bit
    uint8_t dataPart;
#endif
    _rxValue &= 0x7F;  // 패리티 비트 제거

#ifdef SDI12_CHECK_PARITY
    dataPart = _rxValue;
    uint8_t checkParity = parity_even_bit(_rxValue);  // Calculate the parity bit from character w/o parity

    //  ESP_EARLY_LOGW(TAG, "Raw Byte = 0x%02X, BIN = " BYTE_TO_BINARY_PATTERN, _rxValue, BYTE_TO_BINARY(_rxValue));
    //  ESP_EARLY_LOGW(TAG, "DataPart = 0x%02X, ParityBit (recv) = %u, Calculated = %u", dataPart, rxParity,
    //  checkParity);
    ESP_EARLY_LOGW(TAG, "[ISR DEBUG]");
    ESP_EARLY_LOGW(TAG, "  Raw Byte    = 0x%02X, BIN = " BYTE_TO_BINARY_PATTERN, _rxValue, BYTE_TO_BINARY(_rxValue));
    ESP_EARLY_LOGW(TAG, "  Data Part   = 0x%02X", dataPart);
    ESP_EARLY_LOGW(TAG, "  Parity Bit  = recv: %u, calc: %u", rxParity, checkParity);

    // rxState, rxMask 로그
    ESP_EARLY_LOGW(TAG, "  rxState     = %u", _rxState);
    ESP_EARLY_LOGW(TAG, "  rxMask      = 0x%02X, BIN = " BYTE_TO_BINARY_PATTERN, _rxMask, BYTE_TO_BINARY(_rxMask));

    if (rxParity != checkParity) {
      ESP_EARLY_LOGW(TAG, "⚠️ Parity mismatch: recv=%u, calc=%u, val=0x%02X", rxParity, checkParity, dataPart);
      _parityFailure = true;
    }

    if (!_parityFailure) {
#endif
      charToBuffer(_rxValue);

#ifdef SDI12_CHECK_PARITY
    }
#endif
    if (level == HIGH || !nextChar) {
      _rxState = 0xFF;
      /* // 이벤트 비트 세팅 */
      /* BaseType_t hp = pdFALSE; */
      /* xEventGroupSetBitsFromISR(sdi12_event_group, SDI12_EVENT_RX_CHAR, &hp); */
      /* if (hp) */
      /*   portYIELD_FROM_ISR(); */
    } else {
      startChar();
    }
  }

  _prevTimeUs = now;
}

static void setPinInterrupts(bool enable) {
  if (enable) {
    gpio_isr_handler_add(_pinRX, sdi12_receive_isr, NULL);      // 명시적 등록
    gpio_set_intr_type((gpio_num_t)_pinRX, GPIO_INTR_ANYEDGE);  // 타입 명시
    gpio_intr_enable((gpio_num_t)_pinRX);
  } else {
    gpio_isr_handler_remove(_pinRX);
    gpio_intr_disable((gpio_num_t)_pinRX);
  }
}
static void setState(SDI12_State st) {
  _state = st;
  switch (st) {
    case SDI12_HOLDING:
      pinMode(_pinRX, INPUT);
      setPinInterrupts(false);
      pinMode(_pinTX, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTX, LOW);
      pinMode(_pinTXOE, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTXOE, HIGH);
      break;
    case SDI12_TRANSMITTING:
      pinMode(_pinTX, OUTPUT);

      pinMode(_pinRX, INPUT);
      setPinInterrupts(false);

      pinMode(_pinTXOE, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTXOE, LOW);

      break;
    case SDI12_LISTENING:
      pinMode(_pinRX, INPUT);
      setPinInterrupts(true);
      _rxState = 0xFF;
      pinMode(_pinTX, INPUT);
      pinMode(_pinTXOE, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTXOE, HIGH);
      _prevTimeUs = READTIME();  // Set the last interrupt time to now
      break;
    default:
      gpio_set_level((gpio_num_t)_pinRX, HIGH);
      pinMode(_pinRX, INPUT);
      setPinInterrupts(false);
      pinMode(_pinTX, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTX, LOW);
      pinMode(_pinTXOE, OUTPUT);
      gpio_set_level((gpio_num_t)_pinTXOE, HIGH);
      break;
  }
}

static void wakeSensors(int extraWakeTime) {
  setState(SDI12_TRANSMITTING);
  // break
  gpio_set_level((gpio_num_t)_pinTX, HIGH);
  esp_rom_delay_us(LINEBREAK_US);
  // esp_rom_delay_us((uint32_t)3000000);
  //  추가 웨이크
  vTaskDelay(pdMS_TO_TICKS(extraWakeTime));
  // marking
  gpio_set_level((gpio_num_t)_pinTX, LOW);
  // esp_rom_delay_us((uint32_t)3000000);
  esp_rom_delay_us(MARKING_US);
}

void writeChar(uint8_t outChar) {
  uint8_t currentTxBitNum = 0;  // first bit is start bit
  uint8_t bitValue = 1;         // start bit is HIGH (inverse parity...)

  portDISABLE_INTERRUPTS();

  sdi12timer_t t0 = READTIME();
  gpio_set_level((gpio_num_t)_pinTX, HIGH);  // Start bit (inverse logic: HIGH)

  currentTxBitNum++;

  // Parity 추가
  uint8_t parityBit = parity_even_bit(outChar);
  outChar |= (parityBit << 7);

  // 마지막 HIGH 비트 위치 계산 (1의 위치)
  uint8_t lastHighBit = 9;
  uint8_t msbMask = 0x80;
  while (msbMask & outChar) {
    lastHighBit--;
    msbMask >>= 1;
  }

  // start bit 유지
  while ((uint8_t)(READTIME() - t0) < TICKS_PER_BIT) {}
  t0 = READTIME();

  // 데이터/패리티 비트 전송
  while (currentTxBitNum++ < lastHighBit) {
    bitValue = outChar & 0x01;
    gpio_set_level((gpio_num_t)_pinTX, bitValue ? LOW : HIGH);  // 1 → LOW, 0 → HIGH (inverse logic)
    while ((uint8_t)(READTIME() - t0) < TICKS_PER_BIT) {}
    t0 = READTIME();
    outChar = outChar >> 1;  // shift character to expose the following bit
  }

  // 남은 시간 동안 STOP 비트 포함 유지 (LOW)
  gpio_set_level((gpio_num_t)_pinTX, LOW);
  portENABLE_INTERRUPTS();

  uint8_t bitTimeRemaining = TICKS_PER_BIT * (10 - lastHighBit);
  while ((uint8_t)(READTIME() - t0) < bitTimeRemaining) {}
}
