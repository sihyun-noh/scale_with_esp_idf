
#include "sdi12.h"

// 내부 구조체: SDI-12 통신에 필요한 포트/핀 정보
static struct {
    uart_port_t uart_num;
    int tx_pin;
    int rx_pin;
    int oe_pin;
} sdi12;

/* 내부 함수 */
static esp_err_t SDI12_QueryDevice(const char *cmd, uint8_t cmd_len, char *response, uint8_t response_len);
static esp_err_t SDI12_ReceiveLine(char *buffer, uint8_t max, uint8_t *count);

/**
 * @brief SDI-12 초기화 (UART 및 OE 핀)
 */
esp_err_t SDI12_Init(uart_port_t uart_num, int tx_pin, int rx_pin, int oe_pin) {
    sdi12.uart_num = uart_num;
    sdi12.tx_pin = tx_pin;
    sdi12.rx_pin = rx_pin;
    sdi12.oe_pin = oe_pin;

    // UART 설정 (1200 7E1)
    uart_config_t uart_config = {
        .baud_rate = SDI12_BAUD_RATE,
        .data_bits = SDI12_DATA_BITS,
        .parity = SDI12_PARITY,
        .stop_bits = SDI12_STOP_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    esp_err_t err = uart_param_config(sdi12.uart_num, &uart_config);
    if (err != ESP_OK)
        return err;

    // TX, RX 핀 매핑
    err = uart_set_pin(sdi12.uart_num, sdi12.tx_pin, sdi12.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
        return err;

    // UART 버퍼 할당
    err = uart_driver_install(sdi12.uart_num, SDI12_RX_BUF_SIZE, SDI12_TX_BUF_SIZE, 0, NULL, 0);
    if (err != ESP_OK)
        return err;

    // OE 핀: 출력 모드, 기본 Enable (1)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << sdi12.oe_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(sdi12.oe_pin, 1);

    return ESP_OK;
}

/**
 * @brief SDI-12 명령 전송 + 응답 수신
 *
 * 요구 타이밍:
 *  - Break: 라인 High >= 12ms
 *  - Marking: 라인 Low >= 8.33ms
 *  - 명령 전송 후, 센서 응답 (최대 15~150ms 대기)
 *  - 필요한 경우 3회 재시도 가능
 */
static esp_err_t SDI12_QueryDevice(const char *cmd, uint8_t cmd_len, char *response, uint8_t response_len) {
    // === 1. Break: 라인을 High로 12 ms 이상 유지 ===
    // OE=0 → 버퍼 Hi-Z → 외부 풀업으로 라인 High
    gpio_set_level(sdi12.oe_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(12));  // 최소 12 ms

    // === 2. Marking: 라인을 Low로 8.33 ms 이상 유지 ===
    // OE=1 → 버퍼 연결.
    // 단, TX 핀을 GPIO 모드로 바꿔서 직접 LOW 출력 (8.33 ms)
    gpio_set_level(sdi12.oe_pin, 1);
    gpio_set_direction(sdi12.tx_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(sdi12.tx_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(9));  // 9 ms ~ 10 ms

    // TX 핀을 다시 UART 모드로 복원
    uart_set_pin(sdi12.uart_num, sdi12.tx_pin, sdi12.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // === 3. 명령 전송 ===
    //  - 예: "0I!" or "0M!" 등
    //  - 센서 응답은 최대 15 ms 내 수신 (표준 권장),
    //    일부 센서는 더 오래 걸릴 수 있으므로 100 ms 정도 대기 후 재시도
    esp_err_t result = ESP_FAIL;
    const int max_retries = 3;
    for (int attempt = 0; attempt < max_retries; attempt++) {
        // Flush RX 버퍼 (이전 잔류 데이터 제거)
        uart_flush(sdi12.uart_num);

        // 명령 전송
        int bytes_sent = uart_write_bytes(sdi12.uart_num, cmd, cmd_len);
        if (bytes_sent != cmd_len) {
            return ESP_FAIL;
        }

        // === 4. 응답 대기 + 수신 ===
        //    센서가 최대 15~200ms 안에 응답. 센서별 사양 상이
        //    여기서는 110 ms로 설정하고, 재시도 3회
        memset(response, 0, response_len);
        uint8_t count = 0;
        esp_err_t rx_status = SDI12_ReceiveLine(response, response_len, &count);

        if (rx_status == ESP_OK && count > 0) {
            // 정상적으로 데이터를 수신함
            result = ESP_OK;
            break;
        } else {
            // 응답이 없으므로 재시도
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    return result;
}

/**
 * @brief CR/LF로 종료되는 문자열을 센서로부터 수신
 */
static esp_err_t SDI12_ReceiveLine(char *buffer, uint8_t max, uint8_t *count) {
    if (!buffer || max == 0 || !count) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t i = 0;
    while (i < max) {
        // 한 바이트씩 읽으며 최대 110 ms 대기
        int ret = uart_read_bytes(sdi12.uart_num, &buffer[i], 1, pdMS_TO_TICKS(110));
        if (ret <= 0) {
            // 타임아웃 or 에러
            break;
        }
        if (buffer[i] == '\n') {
            // 개행 문자를 받으면 종료
            i++;
            break;
        }
        i++;
    }
    // 수신된 문자열 끝에서 CR/LF 제거
    while (i > 0) {
        char c = buffer[i - 1];
        if (c == '\n' || c == '\r') {
            buffer[i - 1] = '\0';
            i--;
        } else {
            break;
        }
    }
    *count = i;
    return ESP_OK;
}

/* ---- SDI-12 명령 함수들 ---- */

esp_err_t SDI12_AckActive(const char addr) {
    // 예: "0!" → 센서 활성화 확인
    char cmd[3] = { addr, '!', '\0' };
    char response[10] = { 0 };
    return SDI12_QueryDevice(cmd, 2, response, sizeof(response));
}

void SDI12_DevicesOnBus(char *const devices) {
    // '0'~'9' 순회하며 AckActive
    uint8_t idx = 0;
    for (char a = '0'; a <= '9'; a++) {
        if (SDI12_AckActive(a) == ESP_OK) {
            devices[idx++] = a;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

esp_err_t SDI12_GetId(const char addr, char *response, uint8_t response_len) {
    // 예: "0I!"
    char cmd[4] = { addr, 'I', '!', '\0' };
    return SDI12_QueryDevice(cmd, 3, response, response_len);
}

esp_err_t SDI12_ChangeAddr(char *from_addr, char *to_addr) {
    // 예: "0A1!"
    char cmd[5] = { from_addr[0], 'A', to_addr[0], '!', '\0' };
    char resp[10] = { 0 };
    return SDI12_QueryDevice(cmd, 4, resp, sizeof(resp));
}

esp_err_t SDI12_StartMeasurement(const char addr, SDI12_Measure_TypeDef *info) {
    // aM! → atttn
    char cmd[4] = { addr, 'M', '!', '\0' };
    char response[10] = { 0 };
    esp_err_t err = SDI12_QueryDevice(cmd, 3, response, sizeof(response));
    if (err != ESP_OK || response[0] == '\0') {
        return err;
    }

    // response 예: "0xxx3"
    info->Address = response[0];
    // ttt 추출
    char time_buf[4] = { 0 };
    strncpy(time_buf, &response[1], 3);
    info->Time = (uint16_t)atoi(time_buf);
    // n 추출
    info->NumValues = response[4] - '0';
    return ESP_OK;
}

esp_err_t SDI12_SendData(const char addr, const SDI12_Measure_TypeDef *info, char *data) {
    // 측정 데이터 요청: aD0! ~ aD9!
    // 측정값이 여러 개면 D1!, D2! ... 반복
    // 여기서는 D0! ~ D8!까지 시도
    char cmd[5] = { addr, 'D', '0', '!', '\0' };
    uint8_t values_count = 0;
    uint16_t idx = 0;
    data[0] = '\0';

    for (char i = '0'; i <= '8'; i++) {
        cmd[2] = i;
        char response[MAX_RESPONSE_SIZE + 1] = { 0 };
        esp_err_t err = SDI12_QueryDevice(cmd, 4, response, MAX_RESPONSE_SIZE);
        if (err != ESP_OK) {
            return err;
        }
        // 응답 파싱
        // 첫 문자(response[0])는 주소, 그 뒤가 실제 데이터
        uint8_t len = strlen(response);
        if (len > 1) {
            // + 또는 - 기호를 센서값 구분자로 간주
            for (uint8_t x = 1; x < len; x++) {
                if (response[x] == '+' || response[x] == '-') {
                    values_count++;
                }
            }
            // data 버퍼에 이어붙이기
            strncat(data, &response[1], len - 1);
            idx = strlen(data);
        }
        // 모든 값 수신 완료
        if (values_count >= info->NumValues) {
            break;
        }
    }
    return ESP_OK;
}

esp_err_t SDI12_StartVerification(const char addr, SDI12_Measure_TypeDef *info) {
    // aV! → atttn
    char cmd[4] = { addr, 'V', '!', '\0' };
    char response[10] = { 0 };
    esp_err_t err = SDI12_QueryDevice(cmd, 3, response, sizeof(response));
    if (err != ESP_OK || response[0] == '\0') {
        return err;
    }

    info->Address = response[0];
    char time_buf[4] = { 0 };
    strncpy(time_buf, &response[1], 3);
    info->Time = (uint16_t)atoi(time_buf);
    info->NumValues = response[4] - '0';
    return ESP_OK;
}

uint16_t SDI12_CheckCRC(char *response) {
    // SDI-12 1.3 이상에서만 사용
    uint16_t crc = 0;
    for (int i = 0; response[i] != '\0'; i++) {
        crc ^= (uint8_t)response[i];
        for (uint8_t b = 0; b < 8; b++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

esp_err_t SDI12_StartMeasurementCRC(const char addr, SDI12_Measure_TypeDef *info) {
    // aMC! → atttn
    char cmd[5] = { addr, 'M', 'C', '!', '\0' };
    char response[10] = { 0 };
    esp_err_t err = SDI12_QueryDevice(cmd, 4, response, sizeof(response));
    if (err != ESP_OK || response[0] == '\0') {
        return err;
    }

    info->Address = response[0];
    char time_buf[4] = { 0 };
    strncpy(time_buf, &response[1], 3);
    info->Time = (uint16_t)atoi(time_buf);
    info->NumValues = response[4] - '0';

    // 필요 시 response 끝에 붙는 CRC 확인 가능
    SDI12_CheckCRC(response);
    return ESP_OK;
}
