
// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "sdi12_bitbang.h"

static const char *TAG = "example_main";

void sdi12_task(void *arg) {
  // 1) 드라이버 초기화 (RX GPIO 4, TX GPIO 5, TX_OE GPIO 2 예시)
  sdi12_init(4, 5, 2);

  // 2) 통신 시작
  sdi12_begin();

  // 센서 주소 '0'에 대해 측정 명령(M!) 전송
  //   → "0M!" 뒤에 센서가 응답까지 준비하는 시간을 100ms 추가로 기다림
  sdi12_send_command("0M!", 100);

  // 3) EventGroup 으로 수신 이벤트 대기
  EventBits_t bits = xEventGroupWaitBits(sdi12_event_group, SDI12_EVENT_RX_CHAR,
                                         pdTRUE,   // 기다운 후 비트 자동 클리어
                                         pdFALSE,  // 모든 비트가 아니라 단일 비트만
                                         pdMS_TO_TICKS(2000));

  if (bits & SDI12_EVENT_RX_CHAR) {
    // 4) 버퍼에 쌓인 모든 문자를 읽어서 로그로 출력
    char buf[SDI12_BUFFER_SIZE + 1];
    int len = 0;
    while (len < SDI12_BUFFER_SIZE && sdi12_available() > 0) {
      int c = sdi12_read();
      if (c < 0)
        break;
      buf[len++] = (char)c;
    }
    buf[len] = '\0';
    ESP_LOGI(TAG, "Received: %s", buf);
  } else {
    ESP_LOGW(TAG, "Timeout waiting for SDI-12 response");
  }

  // 5) 통신 종료 & 드라이버 해제
  sdi12_end();
  sdi12_deinit();

  vTaskDelete(NULL);
}

void app_main(void) {
  // 하나의 FreeRTOS 태스크로 실행
  xTaskCreate(sdi12_task, "sdi12_task", 4 * 1024, NULL, 5, NULL);
}
