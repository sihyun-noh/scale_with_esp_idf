
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include <sys/time.h>

static const char* TAG = "TIME_SYNC";

// 1) SNTP 초기화
static void initialize_sntp(void) {
  ESP_LOGI(TAG, "Initializing SNTP");
  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  // NTP 서버를 원하는 대로 바꿀 수 있습니다.
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_init();
}

static void initialize_sntp_kst(void) {
  ESP_LOGI(TAG, "Initializing SNTP (KST)");

  if (esp_sntp_enabled()) {
    esp_sntp_stop();
  }
  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org");
  esp_sntp_setservername(1, "time.google.com");
  esp_sntp_setservername(2, "1.pool.ntp.org");
  esp_sntp_init();
  // KST 설정 (Korea Standard Time = UTC+9)
  setenv("TZ", "KST-9", 1);  // POSIX 시간대 형식
  tzset();
}

// 2) 시간 동기화 수행 및 fallback 처리
static void obtain_time(void) {
  // initialize_sntp();
  initialize_sntp_kst();

  // 최대 10회(약 20초) 동안 동기화 대기
  int retry = 0;
  const int retry_count = 10;
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for SNTP sync... (%d/%d)", retry + 1, retry_count);
    vTaskDelay(pdMS_TO_TICKS(2000));
    retry++;
  }

  // 동기화 실패 시 fallback
  if (retry_count <= retry) {
    ESP_LOGW(TAG, "SNTP sync failed! Setting fallback time: 9999-09-99");

    struct tm fb = { .tm_year = 9999 - 1900,  // 년도: year since 1900
                     .tm_mon = 9 - 1,         // 월: 0-based
                     .tm_mday = 99,           // 일자: 의도된 sentinel
                     .tm_hour = 0,
                     .tm_min = 0,
                     .tm_sec = 0 };
    time_t t_fb = mktime(&fb);
    struct timeval tv = { .tv_sec = t_fb, .tv_usec = 0 };
    settimeofday(&tv, NULL);
  }

  // 최종 설정된 시간 로그 출력
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  ESP_LOGI(TAG, "Current time: %04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
           timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// 3) IP 이벤트 핸들러: STA가 IP를 획득하면 시간 동기화 시작

void on_got_ip(void* arg, esp_event_base_t base, int32_t id, void* data) {
  ESP_LOGI(TAG, "Got IP, now attempting SNTP sync %s", base);
  obtain_time();
}

/* void app_main(void) */
/* { */
/*     // NVS 초기화 (Wi-Fi 사용을 위해 필수) */
/*     esp_err_t ret = nvs_flash_init(); */
/*     if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { */
/*         ESP_ERROR_CHECK(nvs_flash_erase()); */
/*         ret = nvs_flash_init(); */
/*     } */
/*     ESP_ERROR_CHECK(ret); */
/**/
/*     // 기본 이벤트 루프 */
/*     ESP_ERROR_CHECK(esp_event_loop_create_default()); */
/**/
/*     // IP 획득 이벤트 등록 */
/*     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL)); */
/**/
/*     // Wi-Fi 스테이션 초기화 */
/*     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); */
/*     ESP_ERROR_CHECK(esp_wifi_init(&cfg)); */
/*     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); */
/*     ESP_ERROR_CHECK(esp_wifi_start()); */
/**/
/*     // 여기서 SSID/PASSWORD 설정 후 esp_wifi_connect() 호출 */
/*     // ex) */
/*     wifi_config_t wifi_cfg = { */
/*         .sta = { */
/*             .ssid = "YOUR_SSID", */
/*             .password = "YOUR_PASS", */
/*         }, */
/*     }; */
/*     ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_cfg)); */
/*     ESP_ERROR_CHECK(esp_wifi_connect()); */
/* } */
