
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "SDI12.h"
#include "sensor_auto_detect.h"
#include "sensor_cfg_config.h"
#include "sensor_config.h"
#include "soc/gpio_num.h"
#include "sdi12_task.h"

#define TX_PIN             CONFIG_SDI12_TX_PIN  // The pin of the SDI-12 data bus.
#define RX_PIN             CONFIG_SDI12_RX_PIN  // The pin of the SDI-12 data bus.
#define OE                 CONFIG_SDI12_OE_PIN  // Output enable pin, active low.
#define MAX_BUF_SDI12_SIZE 120

static const char *TAG = "[sdi12Task]";

/**
 * @brief SDI-12 driver instance for this module.
 *
 * Defined in the SDI-12 task source file (.cpp) and initialized independently.
 * This object is created when the module is loaded, using specified pins for
 * RX, TX, and output enable (OE).
 *
 * Acts as the dedicated SDI-12 interface for this module.
 */
SDI12 mySDI12(RX_PIN, TX_PIN, OE);

/**
 * @brief Static buffer for SDI-12 command and response data.
 *
 * Local to this module. Used to store incoming responses and prepare outgoing commands.
 */
static char buff[MAX_BUF_SDI12_SIZE];

/**
 * @brief SDI-12 Command Template Table
 *
 * This 2-dimensional array maps SDI-12 sensor types to their supported SDI-12 commands.
 * Each command entry contains:
 *   - The SDI-12 command string to be sent (e.g., "I!", "R0!", "M0!", "D0!")
 *   - The expected delay time in milliseconds (how long to wait after sending the command)
 *
 * The table is indexed by:
 *   - First dimension: SENSOR_TYPE (e.g., TEROS11, TEROS12, APOGEE_S2_412)
 *   - Second dimension: SDI_CMD (e.g., SDI_CMD_INFO, SDI_CMD_READ, SDI_CMD_M, SDI_CMD_D)
 *
 * Example usage:
 *   const char *cmd = cmd_table[TEROS12][SDI_CMD_READ].cmd;
 *   uint16_t wait_ms = cmd_table[TEROS12][SDI_CMD_READ].wait_ms;
 *
 * Notes:
 *   - Some sensors only support basic SDI-12 commands (INFO, ADDR, READ).
 *   - Some sensors (e.g., Apogee NDVI sensors) use the M and D command sequence.
 *   - The delay value is important for ensuring sensors have time to prepare their response.
 */
sdi12_cmd_template_t cmd_table[SENSOR_TYPE_COUNT][SDI_CMD_COUNT] = {
  [INFO_INDEX] = {
    [SDI_CMD_INFO]  = { "I!",  100 },
  },
  [TEROS11] = {
    [SDI_CMD_INFO]  = { "I!",  100 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50  },
  },
  [TEROS12] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 100  },
  },
  [TEROS21] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 30 },
  },
  [TEROS54] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50 },
  },
  [ATMOS22] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 30 },
  },
  [ATMOS41] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 20 },
  },
  [APOGEE_S2_411] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50 },
    [SDI_CMD_M]  = { "M2!", 50 },     // read to NDVI measurement command 
    [SDI_CMD_D]  = { "D0!", 50 },
  },
  [APOGEE_S2_412] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50 },
    [SDI_CMD_M]  = { "M2!", 50 },     // read to NDVI measurement command 
    [SDI_CMD_D]  = { "D0!", 50 },
  },
  [APOGEE_SQ_521] = {
    [SDI_CMD_INFO]  = { "I!",  30 },
    [SDI_CMD_ADDR]  = { "A!",  100 },
    [SDI_CMD_READ]  = { "R0!", 50 },
    [SDI_CMD_M]  = { "M0!", 50 },
    [SDI_CMD_D]  = { "D0!", 50 },
  },

};

#if 0
typedef struct {
  const char* (*make_command)(char addr, sdi12_cmd_type_t type);
  bool (*parse_response)(const char* buf, void* out_data);
  void (*print_callback)(const void* data);  
  size_t data_size;  
} sensor_strategy_t;


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
    ESP_LOGE(TAG, "[INFO] Model TER11(TEROS11)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER11");

  } else if (strstr(resp, "TER12") != 0) {
    ESP_LOGE(TAG, "[INFO] Model TER12(TEROS12)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER12");

  } else if (strstr(resp, "TER21") != 0) {
    ESP_LOGE(TAG, "[INFO] Model TER21(TEROS21)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER21");

  } else if (strstr(resp, "TER54") != 0) {
    ESP_LOGE(TAG, "[INFO] Model TER54(TEROS54)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "TER54");

  } else if (strstr(resp, "ATM22") != 0) {
    ESP_LOGE(TAG, "[INFO] Model ATM22(ATMOS22)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "ATMOS22");

  } else if (strstr(resp, "AT41G2") != 0) {
    ESP_LOGE(TAG, "[INFO] Model AT41G2(ATMOS41)");
    strcpy(out->manufacturer, "METER");
    strcpy(out->model, "ATMOS41");

  } else if (strstr(resp, "SQ-521") != 0) {
    ESP_LOGE(TAG, "[INFO] Model SQ-521");
    strcpy(out->manufacturer, "APOGEE");
    strcpy(out->model, "SQ-521");

  } else if (strstr(resp, "S2-412") != 0) {
    ESP_LOGE(TAG, "[INFO] Model S2-412");
    strcpy(out->manufacturer, "APOGEE");
    strcpy(out->model, "S2-412");

  } else if (strstr(resp, "S2-411") != 0) {
    ESP_LOGE(TAG, "[INFO] Model S2-411");
    strcpy(out->manufacturer, "APOGEE");
    strcpy(out->model, "S2-411");

  } else {
    ESP_LOGW(TAG, "[INFO] Not matching model found.");
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

  // e.g. "0+1808.28+26.3"
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

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

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

  // e.g. "0+1808.28+26.3+0"
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

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

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

  // e.g. "0-1808.28+26.3"
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

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

    switch (i) {
      case 0: out->matricPotential = value; break;
      case 1: out->temperature = value; break;
    }

    ptr = endptr;
  }

  return true;
}

static bool parse_teros54_response(const char *resp, teros54_data_t *out) {
  if (!resp || !out)
    return false;

  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 8 float/int values
  for (int i = 0; i < 8; i++) {
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

    switch (i) {
      case 0: out->vwc1 = value; break;
      case 1: out->temp1 = value; break;
      case 2: out->vwc2 = value; break;
      case 3: out->temp2 = value; break;
      case 4: out->vwc3 = value; break;
      case 5: out->temp3 = value; break;
      case 6: out->vwc4 = value; break;
      case 7: out->temp4 = value; break;
    }

    ptr = endptr;
  }

  return true;
}

// atmos22 data parsed
static bool parse_atmos22_response(const char *resp, weather_atmos22_data_t *out) {
  if (!resp || !out)
    return false;

  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 9 float/int values
  for (int i = 0; i < 9; i++) {
    // Skip unexpected characters
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

    switch (i) {
      case 0: out->windSpeed = value; break;
      case 1: out->windDirection = value; break;
      case 2: out->gustWindSpeed = value; break;
      case 3: out->airTemperature = value; break;
      case 4: out->xOrientation = value; break;
      case 5: out->yOrientation = value; break;
      case 6: out->nullValue = value; break;
      case 7: out->northWindSpeed = value; break;
      case 8: out->eastWindSpeed = value; break;
    }

    ptr = endptr;
  }

  return true;
}

// at41g2 data parsed
static bool parse_at41g2_response(const char *resp, weather_at41g2_data_t *out) {
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

    // Handle sensor 'no data' value -9999
    if (value <= -9999.0f) {
      ESP_LOGW(TAG, "Field %d is invalid (-9999). Replacing with 0.0", i);
      return false;
    }

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

// apogee data parsed
static bool parse_apogee_response(const char *resp, apogee_sensor_t *out, sdi12_sensor_type_t type) {
  if (!resp || !out)
    return false;

  // e.g.: " 0+4.8351"
  char *ptr = (char *)resp;

  // 1. Sensor address (1 char)
  out->address = *ptr++;

  // 2. Parse the 1 float/int values
  for (int i = 0; i < 1; i++) {
    if (*ptr != '+' && *ptr != '-')
      return false;  // sign 체크
    char *endptr = NULL;
    float value = strtof(ptr, &endptr);
    if (endptr == ptr)
      return false;

    switch (i) {
      case 0: {
        if (type == APOGEE_SQ_521)
          out->PPFD = value;
        else if (type == APOGEE_S2_411)
          out->NDVI_UP_WARD = value;
        else if (type == APOGEE_S2_412)
          out->NDVI_DOWN_WARD = value;
        break;
      }
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

#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[CMD] Sensor:  %s, Command: %s, Wait: %dms", sensor_type_to_str(SENS_TYPE), full_cmd, delay_ms);
#endif
    mySDI12.clearBuffer();
    mySDI12.sendCommand(full_cmd);  // read data

#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[CMD] Sent command. Waiting for %d ms...", delay_ms);
#endif
    vTaskDelay(pdMS_TO_TICKS(delay_ms));

    int avail = mySDI12.available_esp();
    if (avail > 0) {
      int len = 0;
      ESP_LOGI(TAG, "[RESP] Data available: %d bytes", avail);
      while (mySDI12.available_esp()) {
        int c = mySDI12.read_esp();

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

static char *parse_array_ctrl(char *buf, const char *target) {
  static char *start = NULL;

  start = strstr(buf, target);
  if (!start) {
    ESP_LOGI(TAG, "[RESULTE] Target not found : %s", target);
    return NULL;  // target 문자열이 없으면 NULL 반환
  }

  start += strlen(target);
  return start;
}

static bool print_sdi12_data(char i, sdi12_sensor_type_t SENSOR_TYPE, void *out_data) {
  char addr = i;
  char removeStr[5] = { 0 };
  memset(buff, 0x00, sizeof(buff));

  if (SENSOR_TYPE == APOGEE_SQ_521 || SENSOR_TYPE == APOGEE_S2_411 || SENSOR_TYPE == APOGEE_S2_412) {
    sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_M);
#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[RESULTE] Command \"M\" value : %s", buff);
#endif
    trim(buff);
    vTaskDelay(pdMS_TO_TICKS(1000));
    memset(buff, 0x00, sizeof(buff));
    sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_D);
#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[RESULTE] Value \"D0\" value : %s", buff);
#endif
    trim(buff);
    strncpy(removeStr, "0D0!", 4);

  } else {
    // meter group sensor
    sdi12_get_data(addr, buff, SENSOR_TYPE, SDI_CMD_READ);
#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[RESULTE] Value : %s", buff);
#endif
    // clean up buffer(remove : ' ', '\n', '\r', '\t'))
    trim(buff);
    strncpy(removeStr, "0R0!", 4);
  }

  // Removed because command prefix is included
  char *parsed = parse_array_ctrl(buff, removeStr);
  if (!parsed) {
    ESP_LOGE(TAG, "Parsed response is NULL");
    return false;
  } else {
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "[PARSED] Cleaned result: %s", parsed);
#endif
  }

  switch (SENSOR_TYPE) {
    case TEROS11: {
      teros11_data_t *teros11 = (teros11_data_t *)out_data;
      if (parse_teros11_response(parsed, teros11)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC: %.2f, TEMP: %.2f", teros11->address, (float)teros11->vwc,
                 (float)teros11->temperature);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS12 response.");
        return false;
      }
    }
    case TEROS12: {
      teros12_data_t *teros12 = (teros12_data_t *)out_data;
      if (parse_teros12_response(parsed, teros12)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC: %.2f, TEMP: %.2f, EC: %.2f", teros12->address, (float)teros12->vwc,
                 (float)teros12->temperature, (float)teros12->ec);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS12 response.");
        return false;
      }
    }
    case TEROS21: {
      teros21_data_t *teros21 = (teros21_data_t *)out_data;
      if (parse_teros21_response(parsed, teros21)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, MATRICPOTENTIAL: %.2f, TEMP: %.2f", teros21->address,
                 (float)teros21->matricPotential, (float)teros21->temperature);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS21 response.");
        return false;
      }
    }
    case TEROS54: {
      teros54_data_t *teros54 = (teros54_data_t *)out_data;
      if (parse_teros54_response(parsed, teros54)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, VWC_1: %.2f, TEMP_1: %.2f", teros54->address, (float)teros54->vwc1,
                 (float)teros54->temp1);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse TEROS54 response.");
        return false;
      }
    }
    case ATMOS22: {
      weather_atmos22_data_t *atmos22 = (weather_atmos22_data_t *)out_data;
      if (parse_atmos22_response(parsed, atmos22)) {
        ESP_LOGI(TAG,
                 "[PARSED] ADDR: %c | TEMP: %.2f°C | WIND: %.2f m/s (%.1f°) | GUST: %.2f m/s | "
                 "ORIENTATION: X %.1f° Y %.1f° | N WIND: %.2f m/s | E WIND: %.2f m/s | NULL: %.2f",
                 atmos22->address, atmos22->airTemperature, atmos22->windSpeed, atmos22->windDirection,
                 atmos22->gustWindSpeed, atmos22->xOrientation, atmos22->yOrientation, atmos22->northWindSpeed,
                 atmos22->eastWindSpeed, atmos22->nullValue);

        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse ATMOS22 response.");
        return false;
      }
    }
    case ATMOS41: {
      weather_at41g2_data_t *at41g2 = (weather_at41g2_data_t *)out_data;
      if (parse_at41g2_response(parsed, at41g2)) {
        ESP_LOGI(TAG,
                 "[PARSED] ADDR: %c | TEMP: %.2f°C | RH: %.2f%% | PRES: %.2f hPa | "
                 "WIND: %.2f m/s (%.1f°) | RAIN: %.2f mm",
                 at41g2->address, at41g2->airTemperature, at41g2->relativeHumidity, at41g2->atmosphericPressure,
                 at41g2->windSpeed, at41g2->windDirection, at41g2->precipitation);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse ATMOS41 response.");
        return false;
      }
    }
    case APOGEE_S2_411: {
      apogee_sensor_t *s2_411 = (apogee_sensor_t *)out_data;
      if (parse_apogee_response(parsed, s2_411, APOGEE_S2_411)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, NDVI Upward : %.4f", s2_411->address, s2_411->NDVI_UP_WARD);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse apogee S2-411 response.");
        return false;
      }
    }

    case APOGEE_S2_412: {
      apogee_sensor_t *s2_412 = (apogee_sensor_t *)out_data;
      if (parse_apogee_response(parsed, s2_412, APOGEE_S2_412)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, NDVI Downward : %.4f", s2_412->address, s2_412->NDVI_DOWN_WARD);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse apogee S2-412 response.");
        return false;
      }
    }

    case APOGEE_SQ_521: {
      apogee_sensor_t *sq_521 = (apogee_sensor_t *)out_data;
      if (parse_apogee_response(parsed, sq_521, APOGEE_SQ_521)) {
        ESP_LOGI(TAG, "[PARSED] ADDR: %c, PPFD: %.4f µmol m-2 s-1", sq_521->address, sq_521->PPFD);
        return true;
      } else {
        ESP_LOGE(TAG, "Failed to parse apogee SQ-521 response.");
        return false;
      }
    }

    default: ESP_LOGW(TAG, "Unknown sensor type"); return false;
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

// TEROS11
teros11_data_t *sdi12_read_teros11(uint8_t portId) {
  static teros11_data_t teros11;
  ESP_LOGI(TAG, "[SDI12] Data Read to TEROS11");
  // buffer clear
  memset(&teros11, 0x00, sizeof(teros12_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  if (!print_sdi12_data('0', TEROS11, &teros11)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros11;
}

// TEROS12
teros12_data_t *sdi12_read_teros12(uint8_t portId) {
  static teros12_data_t teros12;
  ESP_LOGI(TAG, "[SDI12] Data Read to TEROS12");
  // buffer clear
  memset(&teros12, 0x00, sizeof(teros12_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  if (!print_sdi12_data('0', TEROS12, &teros12)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros12;
}
// TEROS21
teros21_data_t *sdi12_read_teros21(uint8_t portId) {
  static teros21_data_t teros21;
  ESP_LOGI(TAG, "[SDI12] Data Read to TEROS21");
  // buffer clear
  memset(&teros21, 0x00, sizeof(teros21_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));

  vTaskDelay(pdMS_TO_TICKS(1000));
  mySDI12.begin();
  if (!print_sdi12_data('0', TEROS21, &teros21)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros21;
}

// TEROS54
teros54_data_t *sdi12_read_teros54(uint8_t portId) {
  static uint8_t control_pin_flag = 0;
  static teros54_data_t teros54;
  sensor_system_t *sensorSys = sensor_ctl_get_instance();

  if (sensorSys == NULL) {
    ESP_LOGE(TAG, "sensorSys() returned NULL ");
    return NULL;
  }
  ESP_LOGI(TAG, "[SDI12] Data Read to TEROS54");
  // buffer clear
  memset(&teros54, 0x00, sizeof(teros54_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));

  // NOTE:  Flag indicating whether to control the sensor power
#if 0
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#else
  int detechPin = sensorSys->ports[portId].detectPin;
  int level = gpio_get_level((gpio_num_t)detechPin);

#ifdef SDI12_DEBUG_SET
  ESP_LOGW(TAG, "control pin %d level %d", detechPin, level);
#endif
  //  pin level low is deteched
  //  NOTE:weather station is always turned on
  if (!control_pin_flag) {
    control_pin_flag = 1;
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "Set control pin %d", portId);
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));  // booting delay
  }
#endif

  vTaskDelay(pdMS_TO_TICKS(1000));
  mySDI12.begin();
  if (!print_sdi12_data('0', TEROS54, &teros54)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());

#if 0
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
#else
  // pin level low is deteched
  if (level) {
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
    ESP_LOGW(TAG, "Clear control pin %d", portId);
  }
#endif
  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &teros54;
}

// ATMOS22 is always turned on
weather_atmos22_data_t *sdi12_read_atmos22(uint8_t portId) {
  static uint8_t control_pin_flag = 0;
  static weather_atmos22_data_t atmos22;
  sensor_system_t *sensorSys = sensor_ctl_get_instance();

  if (sensorSys == NULL) {
    ESP_LOGE(TAG, "sensorSys() returned NULL ");
    return NULL;
  }

  ESP_LOGI(TAG, "[SDI12] Data Read to ATMOS22");
  // buffer clear
  memset(&atmos22, 0x00, sizeof(weather_atmos22_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));

  int detechPin = sensorSys->ports[portId].detectPin;
  int level = gpio_get_level((gpio_num_t)detechPin);
#ifdef SDI12_DEBUG_SET
  ESP_LOGW(TAG, "control pin %d level %d", detechPin, level);
#endif
  //  pin level low is deteched
  //  NOTE:weather station is always turned on
  if (!control_pin_flag) {
    control_pin_flag = 1;
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "Set control pin %d", portId);
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));  // booting delay
  }

  vTaskDelay(pdMS_TO_TICKS(1000));
  mySDI12.begin();
  if (!print_sdi12_data('0', ATMOS22, &atmos22)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());

  // pin level low is deteched
  if (level) {
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
    ESP_LOGW(TAG, "Clear control pin %d", portId);
  }

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &atmos22;
}

// ATMOS41G2 is always turned on
weather_at41g2_data_t *sdi12_read_atmos41(uint8_t portId) {
  static uint8_t control_pin_flag = 0;
  static weather_at41g2_data_t atmos41;
  sensor_system_t *sensorSys = sensor_ctl_get_instance();

  if (sensorSys == NULL) {
    ESP_LOGE(TAG, "sensorSys() returned NULL ");
    return NULL;
  }

  ESP_LOGI(TAG, "[SDI12] Data Read to ATMOS41G2");
  // buffer clear
  memset(&atmos41, 0x00, sizeof(weather_at41g2_data_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));

  int detechPin = sensorSys->ports[portId].detectPin;
  int level = gpio_get_level((gpio_num_t)detechPin);
#ifdef SDI12_DEBUG_SET
  ESP_LOGW(TAG, "control pin %d level %d", detechPin, level);
#endif
  //  pin level low is deteched
  //  NOTE:weather station is always turned on
  if (!control_pin_flag) {
    control_pin_flag = 1;
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "Set control pin %d", portId);
#endif
    vTaskDelay(pdMS_TO_TICKS(4000));  // booting delay
  }

  vTaskDelay(pdMS_TO_TICKS(1000));
  mySDI12.begin();
  // NOTE:weather station is always turned on
  if (!print_sdi12_data('0', ATMOS41, &atmos41)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());

  // pin level low is deteched
  if (level) {
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
    ESP_LOGW(TAG, "Clear control pin %d", portId);
  }

  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &atmos41;
}

// SQ-521 is always turned on
apogee_sensor_t *sdi12_read_SQ_521(uint8_t portId) {
  static uint8_t control_pin_flag = 0;
  static apogee_sensor_t apogee_sq_521;
  sensor_system_t *sensorSys = sensor_ctl_get_instance();

  if (sensorSys == NULL) {
    ESP_LOGE(TAG, "sensorSys() returned NULL ");
    return NULL;
  }

  ESP_LOGI(TAG, "[SDI12] Data Read to SQ-521");
  // buffer clear
  memset(&apogee_sq_521, 0x00, sizeof(apogee_sensor_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
#if 0 
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#else
  // NOTE: apogee sensor is always turned on
  int detechPin = sensorSys->ports[portId].detectPin;
  int level = gpio_get_level((gpio_num_t)detechPin);
#ifdef SDI12_DEBUG_SET
  ESP_LOGW(TAG, "control pin %d level %d", detechPin, level);
#endif
  //  pin level low is deteched
  if (!control_pin_flag) {
    control_pin_flag = 1;
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "Set control pin %d", portId);
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));  // booting delay
  }
#endif

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  if (!print_sdi12_data('0', APOGEE_SQ_521, &apogee_sq_521)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
#if 0
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
#else
  // pin level low is deteched
  if (level) {
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
    ESP_LOGW(TAG, "Clear control pin %d", portId);
  }
#endif
  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &apogee_sq_521;
}

// S2-411 and S2-412 (NDVI) sensors are always powered on and cannot be dynamically turned off via GPIO.
apogee_sensor_t *sdi12_read_S2_411_412(uint8_t portId, sdi12_sensor_type_t type) {
  static uint8_t control_pin_flag = 0;
  static apogee_sensor_t apogee_s2_data;
  sensor_system_t *sensorSys = sensor_ctl_get_instance();

  if (sensorSys == NULL) {
    ESP_LOGE(TAG, "sensorSys() returned NULL ");
    return NULL;
  }

  if (type != APOGEE_S2_411 && type != APOGEE_S2_412) {
    ESP_LOGE(TAG, "Unsupported sensor type: %d", type);
    return NULL;
  }

  ESP_LOGI(TAG, "[SDI12] Reading from Apogee %s", (type == APOGEE_S2_411) ? "S2-411" : "S2-412");

  // Clear data buffer
  memset(&apogee_s2_data, 0x00, sizeof(apogee_sensor_t));
  // TODO: This is the control point for buffer and power management
  // Sets the MUX to match the data line and power control for the specified port.
  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));

#if 0
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#else
  // NOTE: apogee sensor is always turned on
  int detechPin = sensorSys->ports[portId].detectPin;
  int level = gpio_get_level((gpio_num_t)detechPin);
#ifdef SDI12_DEBUG_SET
  ESP_LOGW(TAG, "control pin %d level %d", detechPin, level);
#endif
  //  pin level low is deteched
  if (!control_pin_flag) {
    control_pin_flag = 1;
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
#ifdef SDI12_DEBUG_SET
    ESP_LOGW(TAG, "Set control pin %d", portId);
#endif
    vTaskDelay(pdMS_TO_TICKS(1000));  // booting delay
  }
#endif

  vTaskDelay(pdMS_TO_TICKS(450));
  mySDI12.begin();
  if (!print_sdi12_data('0', type, &apogee_s2_data)) {
    ESP_LOGE(TAG, "Sensor data parsing failed for address '0'");
    return NULL;
  }
  mySDI12.end();

  // It disables the connection.
  ESP_ERROR_CHECK(sensor_buffer_disable());
#if 0
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
#else
  // pin level low is deteched
  if (level) {
    ESP_ERROR_CHECK(sensor_control_pin_set(portId, 0));
    ESP_LOGW(TAG, "Clear control pin %d", portId);
  }
#endif
  ESP_LOGI(TAG, "End Search for SDI-12 Devices.");

  return &apogee_s2_data;
}

sdi12_sensor_info_t *sdi12_info_start(uint8_t portId) {
  static sdi12_sensor_info_t info;
  ESP_LOGI(TAG, "Start SDI-12 Devices.");

  ESP_ERROR_CHECK(sensor_buffer_select_port(portId));
  ESP_ERROR_CHECK(sensor_control_pin_set(portId, 1));
  vTaskDelay(pdMS_TO_TICKS(2000));
  mySDI12.begin();
  print_sdi12_info('0', INFO_INDEX, &info);
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
    ESP_LOGI(TAG, "0M3!");
    mySDI12.clearBuffer();
    mySDI12.sendCommand("0M3!");  // read data
    vTaskDelay(pdMS_TO_TICKS(100));

    while (mySDI12.available_esp()) {
      int c = mySDI12.read_esp();
      buf[len++] = c;
#ifdef SDI12_DEBUG_SET
      ESP_LOGI(TAG, "[RESP] Char = '%c' (0x%02X)", c >= 32 ? c : '.', c);
#endif
      vTaskDelay(pdMS_TO_TICKS(10));  // 1 character ~ 7.5ms.
    }
    ESP_LOGI(TAG, "[RESULTE] value : %s", buf);
    len = 0;
    memset(buf, 0x00, sizeof(buf));

    ESP_LOGI(TAG, "4 sec delay");
    vTaskDelay(pdMS_TO_TICKS(4000));

    ESP_LOGI(TAG, "0D0!");
    mySDI12.sendCommand("0D0!");  // read data
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

    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(TAG, "0D1!");
    mySDI12.sendCommand("0D1!");  // read data
    mySDI12.clearBuffer();

    // teros12 delay 87ms for read
    vTaskDelay(pdMS_TO_TICKS(100));

    avail = mySDI12.available_esp();
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

    vTaskDelay(pdMS_TO_TICKS(500));
    ESP_LOGI(TAG, "0D2!");
    mySDI12.sendCommand("0D2!");  // read data
    mySDI12.clearBuffer();

    // teros12 delay 87ms for read
    vTaskDelay(pdMS_TO_TICKS(100));

    avail = mySDI12.available_esp();
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

    mySDI12.end();

    weather_at41g2_data_t weather;
    parse_at41g2_response(buf, &weather);

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
  sdi12_sensor_info_t *info = sdi12_info_start((uint8_t)portId);
  ESP_LOGI(TAG, "Menufanture : %s", info->model);
  while (1) {
    // sdi12_sensor_info_t *info = sdi12_info_start((uint8_t)portId);
    // ESP_LOGI(TAG, "Menufanture : %s", info->model);

    if (strcmp(info->model, "TER12") == 0) {
      teros12_data_t *teros12 = sdi12_read_teros12(portId);
      if (teros12 != NULL) {
        ESP_LOGW(TAG, "[TEROS12] SUCSSES : %.2f", teros12->temperature);
      } else {
        ESP_LOGW(TAG, "[TEROS12] READ FAILED");
      }

    } else if (strcmp(info->model, "TER21") == 0) {
      teros21_data_t *teros21 = sdi12_read_teros21(portId);
      if (teros21 != NULL) {
        ESP_LOGW(TAG, "[TEROS21] SUCSSES : %.2f", teros21->temperature);
      } else {
        ESP_LOGW(TAG, "[TEROS21] READ FAILED");
      }

    } else if (strcmp(info->model, "TER54") == 0) {
      teros54_data_t *teros54 = sdi12_read_teros54(portId);
      if (teros54 != NULL) {
        ESP_LOGW(TAG, "[TEROS54] SUCSSES : %.2f", teros54->temp1);
      } else {
        ESP_LOGW(TAG, "[TEROS54] READ FAILED");
      }

    } else if (strcmp(info->model, "ATMOS22") == 0) {
      weather_atmos22_data_t *weather = sdi12_read_atmos22(portId);
      if (weather != NULL) {
        ESP_LOGW(TAG, "[ATMOS22] Weather station : %.2f", weather->airTemperature);
      } else {
        ESP_LOGW(TAG, "[ATMOS22] READ FAILED");
      }

    } else if (strcmp(info->model, "ATMOS41") == 0) {
      weather_at41g2_data_t *weather = sdi12_read_atmos41(portId);
      if (weather != NULL) {
        ESP_LOGW(TAG, "[ATMOS41] Weather station : %.2f", weather->airTemperature);
      } else {
        ESP_LOGW(TAG, "[ATMOS41] READ FAILED");
      }

    } else if (strcmp(info->model, "SQ-521") == 0) {
      apogee_sensor_t *apogee_sq_521 = sdi12_read_SQ_521(portId);
      if (apogee_sq_521 != NULL) {
        ESP_LOGW(TAG, "[APOGEE_SQ_521] Estimated PPFD value : %.4f µmol m-2 s-1", apogee_sq_521->PPFD);
      } else {
        ESP_LOGW(TAG, "[APOGEE_SQ_521] READ FAILED");
      }

    } else if (strcmp(info->model, "S2-411") == 0) {
      apogee_sensor_t *apogee_s2_411 = sdi12_read_S2_411_412(portId, APOGEE_S2_411);
      if (apogee_s2_411 != NULL) {
        ESP_LOGW(TAG, "[APOGEE_S2_411] Estimated NDVI Upward value : %.4f", apogee_s2_411->NDVI_UP_WARD);
      } else {
        ESP_LOGW(TAG, "[APOGEE_S2_411] READ FAILED");
      }

    } else if (strcmp(info->model, "S2-412") == 0) {
      apogee_sensor_t *apogee_s2_412 = sdi12_read_S2_411_412(portId, APOGEE_S2_412);
      if (apogee_s2_412 != NULL) {
        ESP_LOGW(TAG, "[APOGEE_S2_412] Estimated NDVI Downward value : %.4f", apogee_s2_412->NDVI_DOWN_WARD);
      } else {
        ESP_LOGW(TAG, "[APOGEE_S2_412] READ FAILED");
      }

    }

    else {
      ESP_LOGW("TAG", "[INFO] No matching model found for the given sensor type.");
    }

    ESP_LOGI(TAG, "End Search for SDI-12 Devices.");
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void sdi12_task_init() {
  xTaskCreatePinnedToCore(sdi12_app_main_2, "sdi12_app_main", 4096, NULL, 5, NULL, 1);
}
