// UART2 (예시) + TX=GPIO1, RX=GPIO2, OE=GPIO3
    SDI12_Init(UART_NUM_2, GPIO_TX, GPIO_RX, GPIO_OE);

//센서 주소 검색
char found_devices[11] = {0};
SDI12_DevicesOnBus(found_devices);
// found_devices 예: "013" → 주소 0,1,3 센서 존재


//Identification (aI!)
char id_resp[40] = {0};
SDI12_GetId('0', id_resp, sizeof(id_resp));
// id_resp 예: "013TEROS__TER11__..." 센서 모델/버전 정보


//측정 (aM!) 후 결과 수신 (aD0!)
SDI12_Measure_TypeDef m_info;
SDI12_StartMeasurement('0', &m_info);
// m_info.Time, m_info.NumValues 파싱됨

// 센서가 ttt초 후 측정 완료하므로, ttt * 100ms 등 대기
// 표준은 ttt 단위가 초 단위이나 센서별로 해석이 다를 수 있음
vTaskDelay(pdMS_TO_TICKS(m_info.Time * 1000));

char data_buf[100] = {0};
SDI12_SendData('0', &m_info, data_buf);
// data_buf 예: "+12.34+56.78"



//검증
SDI12_Measure_TypeDef v_info;
SDI12_StartVerification('0', &v_info);
// 필요 시 aD0! 등으로 검증 데이터 수신

4. 주요 포인트

Break & Marking
Break: SDI‑12 라인을 12ms 이상 High 상태
여기서는 OE=0(버퍼 Hi‑Z) + 풀업 → 라인 High
Marking: SDI‑12 라인을 8.33ms 이상 Low 상태
OE=1(버퍼 연결) + TX 핀을 GPIO 모드로 Low 출력
재시도 (Retry)
SDI-12 스펙에 따르면 센서가 15~200ms 내에 응답해야 하며,
응답이 없으면 3회 정도 재시도 가능
측정 대기 (StartMeasurement)
aM! 응답 atttn에서 ttt(초)를 파싱 후, 해당 시간만큼 대기
실제 센서에 따라 ttt가 ms인지 s인지 확인 필요
CRC (1.3 이상)
aMC!로 명령 시 응답에 CRC가 포함될 수 있으며, 필요 시 SDI12_CheckCRC()로 검증
하드웨어 회로
3상 버퍼(OE=1이면 TX→SDI-12, OE=0이면 Hi-Z) + MOSFET/레벨시프터(RX)
SDI-12는 5V 로직이므로, MCU(3.3V) 핀은 5V 내성이 필요하거나 레벨 변환이 필요
위 코드는 SDI‑12의 브레이크(12ms), 마킹(8.33ms), 반이중 전송, 응답 재시도 등의
기본 요구 사항을 반영했으며, 실제 센서마다 약간의 타이밍 여유를 더 줄 수도 있습니다.
(예: 브레이크 15ms, 마킹 10ms 등)

추가로, 특정 센서(예: 테로스 TEROS 11, 캠벨 등)는 응답 속도나 ttt 해석이 다를 수 있으니
실 장비와 테스트하여 타이밍/파싱 로직을 조정하면 됩니다.

이상과 같이 SDI-12를 ESP32 시리즈(ESP-IDF)에서 OE 핀을 활용해 반이중 통신을 구현하는 방법을
예시 코드로 보여드렸습니다. 필요에 따라 세부 타이밍이나 버퍼 크기를 조절해 사용하시면 됩니다.


SDI-12는 반이중(Half-Duplex) 프로토콜이라 한 선(DATA_LINE)에서 TX와 RX를 번갈아 써야 하고,
여러 포트를 지원하려면 MUX와 GPIO 리디렉션이 필수입니다.
이제 질문 주신 “TX를 GPIO로 전환했다가 다시 UART로 복원하는 작업을 반복해도 ESP32-S3에서 문제가 없는지” 살펴볼게요.

✅ 요약 결론
ESP32-S3에서 uart_set_pin()을 반복 호출하는 것은 일반적으로 안전하며 정상 동작합니다.
하지만 너무 빠르고 자주 호출하면 타이밍 문제나 핀 할당 충돌, 글리치가 생길 수 있으므로
다음과 같은 안전 수칙과 최적화 전략을 함께 사용하는 것을 추천드립니다.
📌 원리 설명 – 왜 uart_set_pin()이 필요한가?
uart_set_pin()은 내부적으로 GPIO Matrix를 조작해서,

TX 핀은 UARTx TXD에 연결되고
RX 핀은 UARTx RXD에 연결됩니다.
즉,

TX 핀을 GPIO로 바꿔서 직접 LOW 출력 (마킹)
그 후 다시 uart_set_pin()으로 UART 기능 복원
이 과정을 센서마다 반복하겠다는 거죠?

🧠 잠재적 문제?
자주 바꾸면 GPIO Matrix가 느려질까?
→ 아니요. uart_set_pin()은 단순한 GPIO mux 레지스터 조작이기 때문에 성능 저하 없습니다.
다만 너무 빠르게 빈번히 전환하거나,
실시간 타이밍이 중요한 상황에서 인터럽트와 겹치면 예상치 못한 동작이 생길 수 있습니다.
UART 드라이버에 영향이 있을까?
→ uart_set_pin()은 UART 드라이버와 직접 충돌하지 않지만,
전환 타이밍 중간에 RX 수신이 시작되면 데이터 손실 가능성은 있습니다.
따라서 TX→RX 전환 시 충분한 지연이나 라인 안정화 시간 확보가 중요합니다.
✅ 추천 전략
1. 핀 전환 최소화하기 (가능하면)

// GPIO로 마킹 출력
gpio_set_direction(sdi12.tx_pin, GPIO_MODE_OUTPUT);
gpio_set_level(sdi12.tx_pin, 0);
vTaskDelay(pdMS_TO_TICKS(9));

// → 여기까지 마킹 완료되면, UART TX 핀 복원
gpio_set_direction(sdi12.tx_pin, GPIO_MODE_INPUT);  // 대신 'INPUT'으로 두고 MUX만 복원
uart_set_pin(sdi12.uart_num, sdi12.tx_pin, sdi12.rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
UART 복구는 반드시 마킹 후 한 번만 진행
가능한 경우 마킹 시 GPIO Matrix 직접 조작으로 최적화 가능 (고급 테크닉)
2. 각 포트마다 독립된 상태 유지 (예: 구조체 6개)

typedef struct {
    uart_port_t uart;
    int tx;
    int rx;
    int oe;
    int mux_sel_gpio; // 예: A0~A5 선택용
} SDI12_Port;

SDI12_Port ports[6];  // 포트 6개 관리
MUX 제어와 TX/RX 전환을 이 구조체 단위로 관리하면 안정성 ↑
3. 핵심 마킹 & 전환 루틴을 함수화해서 재사용

void SDI12_Marking(SDI12_Port *port)
{
    // Break → OE=0 → Hi-Z
    gpio_set_level(port->oe, 0);
    vTaskDelay(pdMS_TO_TICKS(12));

    // Marking → OE=1 + TX LOW
    gpio_set_level(port->oe, 1);
    gpio_set_direction(port->tx, GPIO_MODE_OUTPUT);
    gpio_set_level(port->tx, 0);
    vTaskDelay(pdMS_TO_TICKS(9));

    // 복원
    uart_set_pin(port->uart, port->tx, port->rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}
💡 더 고급 최적화
IO MUX 레벨 제어: gpio_matrix_out() / gpio_matrix_in() 등으로 uart_set_pin() 없이도 핀 재배치 가능
마킹을 UART TX 0x00 전송으로 대체: 일부 경우 gpio로 전환하지 않고 uart_write_bytes(..., "\x00", 1)로 마킹 대체 가능 (주의 필요)
핵심 루틴 IRAM_ATTR로 등록: FreeRTOS에서 마이크로초 단위 처리 필요 시
✅ 결론
ESP32-S3에서 uart_set_pin()을 반복 호출하는 건 문제 없습니다.
하지만 6채널 MUX 처리 시 타이밍 오류나 불안정한 동작을 막기 위해:
마킹 전환은 신중하게 처리하고,
구조체화 및 함수화로 관리하면 훨씬 안정적인 시스템이 됩니다.
필요하시면 6채널 관리 구조체와 드라이버 형태 예시도 정리해서 드릴 수 있어요.
계속 진행하고 싶으시면 알려주세요!
