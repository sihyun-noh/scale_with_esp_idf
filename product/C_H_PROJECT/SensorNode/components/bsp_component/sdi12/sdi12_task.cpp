
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "SDI12.h"  // Click to install library: // Click to install library:
#include "sensor_auto_detect.h"
#include "sensor_cfg_config.h"
#include "sdi12_task.h"
#include <cstdint>

#define TX_PIN             CONFIG_SDI12_TX_PIN  // The pin of the SDI-12 data bus.
#define RX_PIN             CONFIG_SDI12_RX_PIN  // The pin of the SDI-12 data bus.
#define OE                 CONFIG_SDI12_OE_PIN  // Output enable pin, active low.
#define MAX_BUF_SDI12_SIZE 120

static const char *TAG = "SDI12_task";
SDI12 mySDI12(RX_PIN, TX_PIN, OE);
static char buff[MAX_BUF_SDI12_SIZE];
// sensor info

sdi12_cmd_template_t cmd_table[SENSOR_TYPE_COUNT][SDI_CMD_COUNT] = {
  [TEROS11] = {
    [SDI_CMD_INFO]  = { "I!",  100 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50  },
  },
  [TEROS12] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 100  },
    [SDI_CMD_R3]  = { "R3!", 100  },
    [SDI_CMD_XO]  = { "XO!", 100  },
  },
  [TEROS21] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 30 },
    [SDI_CMD_R3]  = { "R3!", 100  },
    [SDI_CMD_XO]  = { "XO!", 100  },
  },
  [ATMOS41] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 20 },
    [SDI_CMD_R3]  = { "R3!", 100  },
    [SDI_CMD_XO]  = { "XO!", 100  },
  },
};

#if 0
// 공통 전략 인터페이스 구조체
typedef struct {
  const char* (*make_command)(char addr, sdi12_cmd_type_t type);
  bool (*parse_response)(const char* buf, void* out_data);
  void (*print_callback)(const void* data);  // 센서 데이터 출력 콜백
  size_t data_size;  // out_data 구조 크기
} sensor_strategy_t;


// 전략 테이블 정의
static const sensor_strategy_t sensor_strategies[SENSOR_TYPE_COUNT] = {
  [TEROS12] = {
    .make_command = make_teros12_command,
    .parse_response = parse_teros12_response,
    .print_callback = print_teros12_data,
    .data_size = sizeof(teros12_data_t),
  },
  [ATMOS21] = {
    .make_command = make_atmos21_command,
    .parse_response = parse_atmos21_response,
    .print_callback = print_atmos21_data,
    .data_size = sizeof(atmos_data_t),
  },
  // 다른 센서도 동일하게 추가 가능
};

inline const sensor_strategy_t* get_sensor_strategy(sdi12_sensor_type_t sensor_type) {
  if (sensor_type >= SENSOR_TYPE_COUNT) return NULL;
  return &sensor_strategies[sensor_type];
}

#endif

// clean up buffer
char *trim(char *str) {
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

bool parse_sdi12_info(const char *resp, sdi12_sensor_info_t *out) {
  if (!resp || !out)
    return false;

  if (strstr(resp, "TER11") != 0) {
    ESP_LOGE("TAG", "[INFO] Model TER11");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER11");

  } else if (strstr(resp, "TER12") != 0) {
    ESP_LOGE("TAG", "[INFO] Model TER12");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER12");

  } else if (strstr(resp, "TER21") != 0) {
    ESP_LOGE("TAG", "[INFO] Model TER21");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER21");

  } else if (strstr(resp, "AT41G2") != 0) {
    ESP_LOGE("TAG", "[INFO] Model ATMOS41");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "ATMOS41");
  } else {
    ESP_LOGW("TAG", "[INFO] Not matching model found.");
  }

  // memcpy(out->manufacturer, resp + 1, 7);
  // out->manufacturer[7] = '\0';
  //
  // memcpy(out->model, resp + 9, 6);
  // out->model[6] = '\0';
  //
  // memcpy(out->version, resp + 15, 3);
  // out->version[6] = '\0';
  //
  // memcpy(out->serial, resp + 18, 16);
  // out->serial[16] = '\0';

  return true;
}
static bool parse_teros11_response(const char *resp, teros11_data_t *out) {
  if (!resp || !out)
    return false;

  // 예시 응답: "0+1808.28+26.3"
  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 3 float/int values
  for (int i = 0; i < 2; i++) {
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    switch (i) {
      case 0: out->vwc = value; break;
      case 1: out->temperature = value; break;
    }

    ptr = endptr;
  }

  return true;
}

static bool parse_teros12_response(const char *resp, teros12_data_t *out) {
  if (!resp || !out)
    return false;

  // 예시 응답: "0+1808.28+26.3+0"
  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 3 float/int values
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

static bool parse_teros21_response(const char *resp, teros21_data_t *out) {
  if (!resp || !out)
    return false;

  // 예시 응답: "0-1808.28+26.3"
  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 2 float/int values
  for (int i = 0; i < 2; i++) {
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    switch (i) {
      case 0: out->matricPotential = value; break;
      case 1: out->temperature = value; break;
    }

    ptr = endptr;
  }

  return true;
}

static bool parse_weather_response(const char *resp, weather_at41g2_data_t *out) {
  if (!resp || !out)
    return false;

  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 17 float/int values
  for (int i = 0; i < 17; i++) {
    // Skip unexpected characters
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)

      return false;

    switch (i) {
      case 0: out->solar = value; break;
      case 1: out->precipitation = value; break;
      case 2: out->strikes = (int)value; break;
      case 3: out->strikeDistance = (int)value; break;
      case 4: out->windSpeed = value; break;
      case 5: out->windDirection = value; break;
      case 6: out->gustWindSpeed = value; break;
      case 7: out->airTemperature = value; break;
      case 8: out->vaporPressure = value; break;
      case 9: out->atmosphericPressure = value; break;
      case 10: out->relativeHumidity = value; break;
      case 11: out->humiditySensorTemperature = value; break;
      case 12: out->xOrientation = value; break;
      case 13: out->yOrientation = value; break;
      case 14: out->nullValue = value; break;
      case 15: out->northWindSpeed = value; break;
      case 16: out->eastWindSpeed = value; break;
    }

    ptr = endptr;
  }

  return true;
}

static const sdi12_cmd_template_t *get_cmd_template(sdi12_sensor_type_t sensor, sdi12_cmd_type_t cmd_type) {
  if (sensor >= SENSOR_TYPE_COUNT || cmd_type >= SDI_CMD_COUNT)
    return NULL;
  return &cmd_table[sensor][cmd_type];
}

static void sdi12_get_data(const char addr, char *buf, sdi12_sensor_type_t SENS_TYPE, sdi12_cmd_type_t CMD) {
  static char full_cmd[16];
  const sdi12_cmd_template_t *sen_template = get_cmd_template(SENS_TYPE, CMD);

  if (sen_template) {
    memset(full_cmd, 0x00, sizeof(full_cmd));
    snprintf(full_cmd, sizeof(full_cmd), "%c%s", addr, sen_template->cmd_format);  // 예: "0I!"
    int delay_ms = sen_template->wait_delay_ms;

    ESP_LOGI(TAG, "[CMD] Sensor:  %s, Command: %s, Wait: %dms", sensor_type_to_str(SENS_TYPE), full_cmd, delay_ms);

    mySDI12.clearBuffer();
    mySDI12.sendCommand(full_cmd);  // read data

    ESP_LOGI(TAG, "[CMD] Sent command. Waiting for %d ms...", delay_ms);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));

    int avail = mySDI12.available_esp();
    if (avail > 0) {
      int len = 0;
      ESP_LOGI(TAG, "[RESP] Data available: %d bytes", avail);
      while (mySDI12.available_esp()) {
        int c = mySDI12.read_esp();

        //         buf[len++] = c;
        //
        // #ifdef SDI12_DEBUG_SET
        //         ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
        // #endif
        //         vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.

        // 조건: 제어문자는 '%'로 치환
        if ((c >= 0x20 && c <= 0x7E) || c == 0x0D || c == 0x0A) {
          buf[len++] = c;
        } else {
          buf[len++] = '%';
        }

#ifdef SDI12_DEBUG_SET
        ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", (c >= 0x20 && c <= 0x7E) ? c : '.', c);
#endif
        vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.

        if (len >= MAX_BUF_SDI12_SIZE - 1) {
          ESP_LOGW(TAG, "Buffer full, truncating response.");
          break;
        }
      }
      buf[len] = '\0';
    } else {
      ESP_LOGW(TAG, "[RESP] No response from sensor");
    }

  } else {
    ESP_LOGE(TAG, "[ERR] Command template not found for sensor=%d, cmd_type=%d", SENS_TYPE, CMD);
  }
}

// static char *parse_array_ctrl(char *buf, const char *target) {
//   char *start = strstr(buf, target);
//   int len = strlen(target);
//   char *ptr = (char *)buf;
//
//   if (!start)
//     return NULL;  // "0R0!" 없으면 실패
//
//   start += len;  // "0R0!" 문자열 건너뜀 (길이 4)
//   return *ptr + start;
// }

static char *parse_array_ctrl(char *buf, const char *target) {
  static char *start = NULL;

  start = strstr(buf, target);
  if (!start) {
    ESP_LOGI(TAG, "[RESULTE] Target not found : %s", target);
    return NULL;  // target 문자열이 없으면 NULL 반환
  }

  start += strlen(target);  // target 문자열 길이만큼 건너뜀
  return start;             // 그 이후의 위치 포인터 반환
}

void print_sdi12_data(char i, sdi12_sensor_type_t SENSOR_TYPE, void *out_data) {
  char addr = i;
  memset(buff, 0x00, sizeof(buff));
  sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_READ);
  // sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_R3);
  ESP_LOGI(TAG, "[RESULTE] Value : %s", buff);
  // clean up buffer(remove : ' ', '\n', '\r', '\t'))
  trim(buff);
  char *parsed = parse_array_ctrl(buff, "0R0!");

  switch (SENSOR_TYPE) {
    case TEROS11: {
      teros11_data_t *teros11 = (teros11_data_t *)out_data;
      if (parse_teros11_response(parsed, teros11)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC: %.2f, TEMP: %.2f", teros11->address, (float)teros11->vwc,
                 (float)teros11->temperature);
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS12 response.");
      }
      break;
    }
    case TEROS12: {
      teros12_data_t *teros12 = (teros12_data_t *)out_data;
      if (parse_teros12_response(parsed, teros12)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC: %.2f, TEMP: %.2f, EC: %.2f", teros12->address, (float)teros12->vwc,
                 (float)teros12->temperature, (float)teros12->ec);
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS12 response.");
      }
      break;
    }
    case TEROS21: {
      teros21_data_t *teros21 = (teros21_data_t *)out_data;
      if (parse_teros21_response(parsed, teros21)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, MATRICPOTENTIAL: %.2f, TEMP: %.2f", teros21->address,
                 (float)teros21->matricPotential, (float)teros21->temperature);
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS21 response.");
      }
      break;
    }
    case ATMOS41: {
      weather_at41g2_data_t *at41g2 = (weather_at41g2_data_t *)out_data;
      if (parse_weather_response(parsed, at41g2)) {
        ESP_LOGI(TAG,
                 "[PARSED] ADDR: %c | TEMP: %.2f°C | RH: %.2f%% | PRES: %.2f hPa | "
                 "WIND: %.2f m/s (%.1f°) | RAIN: %.2f mm",
                 at41g2->address, at41g2->airTemperature, at41g2->relativeHumidity, at41g2->atmosphericPressure,
                 at41g2->windSpeed, at41g2->windDirection, at41g2->precipitation);
      } else {
        ESP_LOGE(TAG, "Failed to parse ATMOS41 response.");
      }
      break;
    }

    default: ESP_LOGW(TAG, "Unknown sensor type"); break;
  }
}

void print_sdi12_info(char i, sdi12_sensor_type_t SENSOR_TYPE, sdi12_sensor_info_t *info) {
  char addr = i;
  memset(buff, 0x00, sizeof(buff));
  sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_INFO);
  ESP_LOGW(TAG, "[RESULTE] value : %s", buff);
  // rim(buff);
  if (parse_sdi12_info(buff, info)) {
    ESP_LOGE(TAG, "Manufacturer: %s", info->manufacturer);
    ESP_LOGE(TAG, "Model: %s", info->model);
    ESP_LOGE(TAG, "Version: %s", info->version);
    ESP_LOGE(TAG, "Serial: %s", info->serial);
  } else {
    ESP_LOGE(TAG, "Failed to parse SDI-12 info.\n");
  }
}

// TEROS11 READ
teros11_data_t *sdi12_read_start_teros11(uint8_t portId) {
  static teros11_data_t teros11;
  ESP_LOGI(TAG, "[SDI12] Data Read");
  // buffer clear
  memset(&teros11, 0x00, sizeof(teros12_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  print_sdi12_data('0', TEROS11, &teros11);
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros11;
}

// TEROS12 READ
teros12_data_t *sdi12_read_start_teros12(uint8_t portId) {
  static teros12_data_t teros12;
  ESP_LOGI(TAG, "[SDI12] Data Read");
  // buffer clear
  memset(&teros12, 0x00, sizeof(teros12_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  print_sdi12_data('0', TEROS12, &teros12);
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros12;
}
// TEROS21 READ
teros21_data_t *sdi12_read_start_teros21(uint8_t portId) {
  static teros21_data_t teros21;
  ESP_LOGI(TAG, "[SDI12] Data Read");
  // buffer clear
  memset(&teros21, 0x00, sizeof(teros21_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(1000));
  mySDI12.begin();
  print_sdi12_data('0', TEROS21, &teros21);
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros21;
}

// TEROS41G2 READ
weather_at41g2_data_t *sdi12_read_start_teros41(uint8_t portId) {
  static weather_at41g2_data_t teros41;
  ESP_LOGI(TAG, "[SDI12] Data Read");
  // buffer clear
  memset(&teros41, 0x00, sizeof(weather_at41g2_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
  vTaskDelay(pdMS_TO_TICKS(4000));
  mySDI12.begin();
  print_sdi12_data('0', ATMOS41, &teros41);
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros41;
}

sdi12_sensor_info_t *sdi12_info_start(uint8_t portId) {
  static sdi12_sensor_info_t info;
  ESP_LOGI(TAG, "Start SDI-12 Devices.");

  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
  vTaskDelay(pdMS_TO_TICKS(2000));
  mySDI12.begin();
  print_sdi12_info('0', TEROS21, &info);
  mySDI12.end();

  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
  return &info;
}

void sdi12_app_main(void *param) {
  // gpio_set_level((gpio_num_t)1, 0);  // buffer
  //
  // gpio_set_level((gpio_num_t)2, 0);  // S0
  // gpio_set_level((gpio_num_t)3, 0);  // S1
  // gpio_set_level((gpio_num_t)4, 0);  // S2
  //
  // gpio_set_level((gpio_num_t)38, 1);  // powe//
  //
  // Initialize Serial for debug output.

  ESP_LOGI(TAG, "Start");

  char buf[120] = { 0 };
  int len = 0;
  sdi12_sensor_info_t info;
  teros21_data_t teros21;
  gpio_set_level((gpio_num_t)38, 1);  // power port1 on
  ESP_ERROR_CHECK(sensor_buffer_select_port(0));

  while (1) {
    len = 0;
    memset(buf, 0x00, sizeof(buf));
    ESP_LOGI(TAG, "Start SDI-12 Devices.");
    // gpio_set_level((gpio_num_t)38, 1);  // power port1 on
    vTaskDelay(pdMS_TO_TICKS(2000));

    mySDI12.begin();
    print_sdi12_info('0', TEROS12, &info);
    mySDI12.end();

    //     // if (strcmp(info.model, "AT41G2") == 0) {
    //     // if (strstr(info.model, "AT41G2") != NULL) {
    //     if (strstr(buff, "AT41G2") != NULL) {
    //       ESP_LOGI(TAG, "The string 'AT41G2' was detected in the response!");
    //       char cmd_buf[5] = { 0 };
    //
    //       mySDI12.begin();
    //       ESP_LOGI(TAG, "0M!");
    //       mySDI12.clearBuffer();
    //       mySDI12.sendCommand("0M!");  // read data
    //       vTaskDelay(pdMS_TO_TICKS(100));
    //
    //       while (mySDI12.available_esp()) {
    //         int c = mySDI12.read_esp();
    // #ifdef SDI12_DEBUG_SET
    //         ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
    // #endif
    //         vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.
    //       }
    //
    //       ESP_LOGI(TAG, "4 sec delay");
    //       vTaskDelay(pdMS_TO_TICKS(4000));
    //
    //       for (int i = 0; i < 3; i++) {
    //         memset(cmd_buf, 0x00, sizeof(cmd_buf));
    //         snprintf(cmd_buf, sizeof(cmd_buf), "0D%d!", i);
    //
    //         ESP_LOGI(TAG, "%s", cmd_buf);
    //         mySDI12.sendCommand(cmd_buf);  // read data
    //         mySDI12.clearBuffer();
    //         vTaskDelay(pdMS_TO_TICKS(50));
    //
    //         int avail = mySDI12.available_esp();
    //         if (avail > 0) {
    //           ESP_LOGI(TAG, "[RESP] Data available: %d bytes", avail);
    //           while (mySDI12.available_esp()) {
    //             int c = mySDI12.read_esp();
    //             buf[len++] = c;
    //
    // #ifdef SDI12_DEBUG_SET
    //             ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
    // #endif
    //             vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.
    //           }
    //         } else {
    //           ESP_LOGW(TAG, "[RESP] No response from sensor");
    //         }
    //
    //         mySDI12.end();
    //         ESP_LOGI(TAG, "[RESULTE] value : %s", buf);
    //         /*----------------------------------------- */
    //         /*----------------------------------------- */
    //         vTaskDelay(pdMS_TO_TICKS(100));
    //       }
    //       continue;
    //     } else {
    //       ESP_LOGI(TAG, "The string 'AT41G2' was not found in the response.");
    //     }

    mySDI12.begin();
    ESP_LOGI(TAG, "0R0!");
    mySDI12.sendCommand("0R0!");  // read data
    mySDI12.clearBuffer();

    // teros12 delay 87ms for read
    vTaskDelay(pdMS_TO_TICKS(100));

    int avail = mySDI12.available_esp();
    if (avail > 0) {
      ESP_LOGI(TAG, "[RESP] Data available: %d bytes", avail);
      while (mySDI12.available_esp()) {
        int c = mySDI12.read_esp();
        buf[len++] = c;

#ifdef SDI12_DEBUG_SET
        ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
#endif
        vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.
      }
    } else {
      ESP_LOGW(TAG, "[RESP] No response from sensor");
    }

    ESP_LOGI(TAG, "[RESULTE] value : %s", buf);
    weather_at41g2_data_t weather;
    parse_weather_response(buf, &weather);
    mySDI12.end();
    // ESP_LOGI(TAG, "%.2f, %.2f", teros21.matricPotential, teros21.temperature);
    ESP_LOGI(TAG, "%.2f, %.2f", weather.airTemperature, weather.atmosphericPressure);
    ESP_LOGI(TAG, "End Search for SDI-12 Devices.");
    // gpio_set_level((gpio_num_t)38, 0);  // power off
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void sdi12_app_main_2(void *param) {
  ESP_LOGI(TAG, "Start main 2");

  uint8_t portId = 0;
  while (1) {
    sdi12_sensor_info_t *info = sdi12_info_start((uint8_t)portId);
    ESP_LOGI(TAG, "Menufanture : %s", info->model);

    if (strcmp(info->model, "TER12") == 0) {
      teros12_data_t *teros12 = sdi12_read_start_teros12(portId);
      ESP_LOGE(TAG, "[TEROS12] SUCSSES : %.2f", teros12->temperature);
    } else if (strcmp(info->model, "TER21") == 0) {
      teros21_data_t *teros21 = sdi12_read_start_teros21(portId);
      ESP_LOGE(TAG, "[TEROS21] SUCSSES : %.2f", teros21->temperature);
    } else if (strcmp(info->model, "ATMOS41") == 0) {
      weather_at41g2_data_t *weather = sdi12_read_start_teros41(portId);
      ESP_LOGE(TAG, "[TEROS41] weather station : %.2f", weather->airTemperature);
    } else {
      ESP_LOGW("TAG", "[INFO] No matching model found for the given sensor type.");
    }

    ESP_LOGI(TAG, "End Search for SDI-12 Devices.");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void sdi12_task_init() {
  xTaskCreatePinnedToCore(sdi12_app_main_2, "sdi12_app_main", 4096, NULL, 5, NULL, 1);
}
