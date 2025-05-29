
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SDI12.h"  // Click to install library: // Click to install library:
#include "sensor_cfg_config.h"
#define TX_PIN 17  // The pin of the SDI-12 data bus.
#define RX_PIN 18  // The pin of the SDI-12 data bus.
#define OE     16  // Output enable pin, active low.

typedef enum {
  SDI_CMD_INFO = 0,
  SDI_CMD_ADDR,
  SDI_CMD_READ,
  SDI_CMD_COUNT  // 명령 개수
} sdi12_cmd_type_t;

typedef struct {
  const char *cmd_format;  // "I!", "R0!" 등
  int wait_delay_ms;       // 100, 50 등
} sdi12_cmd_template_t;

sdi12_cmd_template_t cmd_table[SENSOR_TYPE_COUNT][SDI_CMD_COUNT] = {
  [TEROS11] = {
    [SDI_CMD_INFO]  = { "I!",  100 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50  },
  },
  [TEROS12] = {
    [SDI_CMD_INFO]  = { "I!",  50 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 100  },
  },
};

typedef struct {
  char address;
  float vwc;
  float temperature;
  float ec;
} teros12_data_t;

static const char *TAG = "SDI12_task";
SDI12 mySDI12(RX_PIN, TX_PIN, OE);

static char buff[100];
const char *sensor_names[] = {
  "TEROS11",       "TEROS12", "TEROS14", "TEROS21",       "ATMOS21",       "ATMOS22",
  "ATMOS31",       "ATMOS41", "ATMOS54", "APOGEE_S2_411", "APOGEE_SP_421", "APOGEE_SQ_521",
  "APOGEE_SU_221",  // ...
};

// clean up buffer
static char *trim(char *str) {
  char *end;

  // 좌측 공백 제거
  while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')
    str++;

  if (*str == 0)  // 전부 공백이면
    return str;

  // 우측 공백 제거
  end = str + strlen(str) - 1;
  while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
    end--;

  // NULL 종료
  *(end + 1) = '\0';

  return str;
}

static bool parse_teros12_response_value(const char *resp, teros12_data_t *out) {
  if (!resp || !out)
    return false;

  // 예시 응답: "0+1808.28+26.3+0"
  char *ptr = (char *)resp;

  // 1. 주소 (첫 번째 문자)
  out->address = *ptr++;

  // 2. 각각의 float 값 파싱
  for (int i = 0; i < 3; i++) {
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    switch (i) {
      case 0: out->vwc = value; break;
      case 1: out->temperature = value; break;
      case 2: out->ec = value; break;
    }

    ptr = endptr;
  }

  return true;
}
const sdi12_cmd_template_t *get_cmd_template(sdi12_sensor_type_t sensor, sdi12_cmd_type_t cmd_type) {
  if (sensor >= SENSOR_TYPE_COUNT || cmd_type >= SDI_CMD_COUNT)
    return NULL;
  return &cmd_table[sensor][cmd_type];
}

static void sdi12_get_template(const char addr, char *buf, sdi12_sensor_type_t SENS_TYPE, sdi12_cmd_type_t CMD) {
  static char full_cmd[16];
  const sdi12_cmd_template_t *sen_template = get_cmd_template(SENS_TYPE, CMD);

  if (sen_template) {
    memset(full_cmd, 0x00, sizeof(full_cmd));
    snprintf(full_cmd, sizeof(full_cmd), "%c%s", addr, sen_template->cmd_format);  // 예: "0I!"
    int delay_ms = sen_template->wait_delay_ms;

    ESP_LOGI(TAG, "[CMD] Sensor:  %s, Command: %s, Wait: %dms", sensor_names[SENS_TYPE], full_cmd, delay_ms);

    mySDI12.clearBuffer();
    mySDI12.sendCommand(full_cmd);

    ESP_LOGI(TAG, "[CMD] Sent command. Waiting for %d ms...", delay_ms);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));

    int avail = mySDI12.available_esp();
    if (avail > 0) {
      int len = 0;
      ESP_LOGI(TAG, "[RESP] Data available: %d bytes", avail);
      while (mySDI12.available_esp()) {
        int c = mySDI12.read_esp();
        buf[len++] = c;
        ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
        vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.
      }
      buf[len] = '\0';
    } else {
      ESP_LOGW(TAG, "[RESP] No response from sensor");
    }

  } else {
    ESP_LOGE(TAG, "[ERR] Command template not found for sensor=%d, cmd_type=%d", SENS_TYPE, CMD);
  }
}

void printInfo(char i) {
  char addr = i;
  memset(buff, 0x00, sizeof(buff));
  sdi12_get_template(addr, buff, TEROS12, SDI_CMD_READ);
  ESP_LOGI(TAG, "[RESULTE] Value : %s", buff);

  // clean up buffer(remove : ' ', '\n', '\r', '\t'))
  trim(buff);

  teros12_data_t parsed;
  if (parse_teros12_response_value(buff, &parsed)) {
    ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC: %.2f, TEMP: %.2f, EC: %.2f", parsed.address, parsed.vwc, parsed.temperature,
             parsed.ec);
  } else {
    ESP_LOGE(TAG, "Failed to parse TEROS12 response.");
  }
}
void sdi12_app_main(void *param) {
  gpio_set_level((gpio_num_t)1, 0);  // buffer

  gpio_set_level((gpio_num_t)2, 1);  // S0
  gpio_set_level((gpio_num_t)3, 0);  // S1
  gpio_set_level((gpio_num_t)4, 0);  // S2

  gpio_set_level((gpio_num_t)39, 1);  // power

  // Initialize Serial for debug output.

  ESP_LOGI(TAG, "Start");

  while (1) {
    gpio_set_level((gpio_num_t)39, 1);  // power
    vTaskDelay(pdMS_TO_TICKS(2000));
    ;

    mySDI12.begin();
    vTaskDelay(pdMS_TO_TICKS(500));

    printInfo('0');
    //  scanAddressSpace();
    mySDI12.end();

    ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

    gpio_set_level((gpio_num_t)39, 0);  // power
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void sdi12_task_init() {
  xTaskCreate(sdi12_app_main, "sdi12_app_main", 4096, NULL, 10, NULL);
}
