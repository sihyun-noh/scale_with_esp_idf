
# SDI-12 Sensor Integration Project - README

## 📌 프로젝트 개요

ESP32-S3 기반의 환경 센서 노드에서 SDI-12 프로토콜을 소프트웨어(bit-bang) 방식으로 구현하여 다양한 SDI-12 센서(Teros12 등)를 자동 감지하고 명령어 기반으로 제어하는 시스템을 개발했습니다.

## 🔧 주요 기능

* GPIO 인터럽트를 통한 센서 연결 감지 (플러그 앤 플레이)
* Bit-banging 방식으로 SDI-12 RX/TX 구현
* 센서 주소 및 명령어 기반 통신 (`I!`, `R0!` 등)
* 커맨드 템플릿 기반 파라미터 매핑 시스템
* JSON-like 파싱을 위한 응답 파서
* `freertos`, `esp_timer`, `esp_log` 활용

## 📁 프로젝트 구조 (핵심 부분)

```plaintext
components/
└── bsp_component/
    └── sdi12/
        ├── SDI12.cpp / SDI12.h         # SDI-12 통신 구현
        ├── sdi12_bitbang_driver.c/h    # 비트뱅 방식 TX/RX 구현
        ├── sensor_auto_detect.c/h      # 센서 연결 감지 로직
        └── command_template.c/h        # 센서별 명령어 매핑 테이블
```

## 📄 주요 구조체

```c
// 센서 타입 정의
typedef enum {
  TEROS11, TEROS12, ..., APOGEE_SU_221,
} sdi12_sensor_type_t;

// 명령어 타입 정의
typedef enum {
  SDI_CMD_INFO, SDI_CMD_READ, SDI_CMD_ADDR,
} sdi12_cmd_type_t;

// 명령어 템플릿 정의
typedef struct {
  const char *cmd_format;    // "I!", "R0!" 등
  uint16_t wait_delay_ms;    // 응답 대기 시간
} sdi12_cmd_template_t;
```

## 🧠 주요 동작 흐름

1. **센서 감지**: GPIO 인터럽트를 통해 센서 연결 여부를 감지 (HIGH/LOW 신호)
2. **명령어 매핑**: 센서 종류 및 요청 타입에 따라 `get_cmd_template(sensor, type)` 로 명령어/대기 시간 획득
3. **명령 전송 및 응답**:

   * `mySDI12.sendCommand(cmd)`
   * 응답 대기 `vTaskDelay()`
   * `mySDI12.read_esp()`로 데이터 수신
4. **응답 파싱**:

   * 예: `0+1808.28+26.3+0` → 주소, VWC, 온도, EC로 구분

## 🧪 예시 응답 파싱

```c
char *resp = trim(buff);  // 경계 제거

parsed.addr = resp[0];
parsed.vwc  = strtof(&resp[1], &p1);
parsed.temp = strtof(p1, &p2);
parsed.ec   = strtof(p2, NULL);
```

## 🐞 디버깅 로깅 예시

```c
ESP_LOGI(TAG, "[CMD] Sensor: TEROS12, Command: %s, Wait: %dms", full_cmd, delay_ms);
ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
```

## ⚠️ 이슈 및 고려사항

* `Arduino.h` 제거 시 `Stream`, `String`, `millis()` 등 제거 필요
* `available()` 함수는 아두이노 의존성 있으므로, 자체 `available_esp()` 함수 구현 필요
* `CMakeLists.txt`에서 `INCLUDE_DIRS`에 .h 파일 직접 지정 ❌ → 디렉토리 지정으로 수정 필요

## ✅ 향후 계획

* JSON 형식으로 응답 포맷 정리 및 MQTT 전송 구조화
* SDI-12 외 RS-485 센서 통합
* OTA 및 장치 등록 시스템 연동

---

📁 문서화 및 관리: Obsidian 기반 `.md` 문서로 버전 관리 중
