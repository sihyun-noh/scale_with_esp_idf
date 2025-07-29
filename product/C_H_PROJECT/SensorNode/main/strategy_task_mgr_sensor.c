
// ===========================

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "cJSON.h"

#include "mqtt_config.h"
#include "mqtt_publish.h"
#include "file_log.h"
#include "littlefs_impl.h"
#include "sdi12_task.h"
#include "sensor_cfg_config.h"
#include "data_table_task.h"
#include "sensor_auto_detect.h"
#include "sensor_config.h"

static const char* TAG = "[sensorStrategy]";
#define MAX_SENSOR_TASKS SENSOR_PORT_COUNT

#define TASK_TRIGGER_BIT BIT0  // TRIGGER_BIT: 태스크 시작을 위한 트리거 비트
#define TASK_DONE_BIT    BIT1  // DONE_BIT: 태스크 완료 알림 비트

EventGroupHandle_t done_group;  ///< FSM waits on this group for task completion (e.g. DONE_BIT)

// @brief
static sensor_port_cfg_t* cfg = NULL;
static sensor_datatable_t* dt = NULL;

// @brief Strategy function type (sensor action execution function)
typedef void (*sensor_action_fn_t)(void* param);

/*
// Example table structure for storing sensor data.
static const sensor_strategy_entry_t sensor_strategies[] = {
  { SENSOR_TYPE_TEROS11, BIT0, handle_teros11, "Teros11Task" },
  { SENSOR_TYPE_PYRANOMETER, BIT1, handle_pyranometer, "PyranoTask" },
  { SENSOR_TYPE_CO2, BIT2, handle_co2, "CO2Task" },
};
*/

/// @brief Structure defining a sensor strategy entry.
typedef struct {
  uint8_t port;               ///< Sensor port number
  sdi12_sensor_type_t type;   ///< Sensor type
  EventBits_t trigger_bit;    ///< Event bit the task waits for
  sensor_action_fn_t action;  ///< Strategy function to execute
  const char* task_name;      ///< Name of the task to create
} sensor_strategy_entry_t;

typedef struct {
  TaskHandle_t handle;               ///< FreeRTOS task handle
  EventGroupHandle_t trigger_group;  ///< FSM triggers the task using this group (e.g. TRIGGER_BIT)
  const char* task_name;             ///< Human-readable task name for logging/debugging
  EventBits_t bit;                   ///< Unique bit per task (shared for trigger/done or separate if needed)
  sensor_action_fn_t action;         ///< Function pointer for sensor read/processing routine

  uint8_t port;            ///< Port number this task is bound to
  sensor_port_cfg_t* cfg;  ///< Sensor configuration pointer
  sensor_datatable_t* dt;  ///< Pointer to associated data-table structure
} sensor_task_t;

/**
 * @brief Creates tasks based on the strategy table.
 *
 * @return true on success, false on failure.
 */
bool strategy_create_tasks(void);

/**
 * @brief Triggers the strategy corresponding to the sensor type.
 *
 * @param type Sensor type (e.g., SENSOR_TYPE_TEROS11).
 * @param status Trigger status indicator.
 */
void sensor_strategy_trigger(sdi12_sensor_type_t type, bool status);

/** @brief Array of registered sensor strategy entries. */
static sensor_strategy_entry_t sensor_strategies[6] = { 0 };

/** @brief Actual number of registered strategies. */
static int strategy_entry_count = 0;

// =============================
// helper function
// =============================

static inline int NUM_SENSOR_STRATEGIES() {
  return (sizeof(sensor_strategies) / sizeof(sensor_strategies[0]));
}

static char* parse_rows(const char* json_text) {
  // 1. JSON 파싱
  static char str[250];
  memset(str, 0, sizeof(str));
  cJSON* root = cJSON_Parse(json_text);
  if (!root) {
    ESP_LOGI(TAG, "Failed to parse JSON\n");
    return NULL;
  }

  // 2. "rows" 배열 가져오기
  cJSON* rows = cJSON_GetObjectItemCaseSensitive(root, "rows");
  if (!cJSON_IsArray(rows)) {
    ESP_LOGI(TAG, "\"rows\" is not an array\n");
    cJSON_Delete(root);
    return NULL;
  }

  // 3. 각 행(row) 처리
  cJSON* row = NULL;
  cJSON_ArrayForEach(row, rows) {
    if (!cJSON_IsArray(row)) {
      continue;
    }

    ESP_LOGI(TAG, "Row: ");
    cJSON* value = NULL;
    cJSON_ArrayForEach(value, row) {
      if (cJSON_IsNumber(value)) {
        ESP_LOGI(TAG, "%.3f ", value->valuedouble);
        sprintf(str + strlen(str), "%.3f,", value->valuedouble);
      } else if (cJSON_IsString(value)) {
        ESP_LOGI(TAG, "%s, ", value->valuestring);
        sprintf(str + strlen(str), "%s,", value->valuestring);
      }
    }
  }
  // 마지막 쉼표 제거
  size_t len = strlen(str);
  if (len > 0 && str[len - 1] == ',') {
    str[len - 1] = '\0';
  }
  cJSON_Delete(root);
  return str;
}

// =============================
// NOTE:
// strategy table define
// Even though the same function is shared across multiple tasks,
// there is no risk of resource contention because the FSM calls each task sequentially.
// Therefore, no additional synchronization mechanisms like mutexes or semaphores are required.
// =============================
static void handle_teros11(void* param) {
  ESP_LOGI(TAG, "[teros11] Handling TEROS11...");
  xEventGroupSetBits(done_group, TASK_DONE_BIT);
  vTaskDelay(pdMS_TO_TICKS(2000));
}

static void handle_teros_series(void* param) {
  int ret = 0;
  char buf[250] = { 0 };
  char topic[MAX_TOPIC_LEN] = { 0 };

  if (!param) {
    ESP_LOGE(TAG, "[TEROS12] Invalid param (NULL)");
    return;
  }

  sensor_task_t* config = (sensor_task_t*)param;
  sensor_datatable_t* dt = config->dt;
  // NOTE: starts at array address 0
  int array_num = config->port - 1;

  const char* sensor_type = sensor_type_to_str(config->cfg[array_num].sensor_type);
  // const char* sensor_type = sensor_type_to_str(TEROS12);
  if (strcmp(sensor_type, "UNKNOWN") == 0) {
    ESP_LOGW(TAG, "Unrecognized sensor type: %s", sensor_type);
  }
  ESP_LOGI(TAG, "[%s] Starting SDI-12 read...port[%d] ", sensor_type, config->port);

  teros12_data_t* data_teros11 = NULL;
  teros12_data_t* data_teros12 = NULL;
  teros21_data_t* data_teros21 = NULL;
  teros54_data_t* data_teros54 = NULL;

  if (config->cfg[array_num].sensor_type == TEROS11) {
    data_teros11 = sdi12_read_teros11(array_num);  // 데이터 읽기
    if (!data_teros11) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      return;
    }
    ESP_LOGI(TAG, "[%s] Raw data - VWC: %.2f, Temp: %.2f", sensor_type, data_teros11->vwc, data_teros11->temperature);
  } else if (config->cfg[array_num].sensor_type == TEROS12) {
    data_teros12 = sdi12_read_teros12(array_num);  // 데이터 읽기
    if (!data_teros12) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      return;
    }
    ESP_LOGI(TAG, "[%s] Raw data - VWC: %.2f, Temp: %.2f, EC: %.2f", sensor_type, data_teros12->vwc,
             data_teros12->temperature, data_teros12->ec);
  } else if (config->cfg[array_num].sensor_type == TEROS21) {
    data_teros21 = sdi12_read_teros21(array_num);  // 데이터 읽기
    if (!data_teros21) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      return;
    }
    ESP_LOGI(TAG, "[%s] Raw data - matricPotential: %.2f, Temp: %.2f", sensor_type, data_teros21->matricPotential,
             data_teros21->temperature);
  } else if (config->cfg[array_num].sensor_type == TEROS54) {
    data_teros54 = sdi12_read_teros54(array_num);  // 데이터 읽기
    if (!data_teros54) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      return;
    }
    ESP_LOGI(TAG, "[%s] Raw data - Vwc1: %.2f, Temp1: %.2f", sensor_type, data_teros54->vwc1, data_teros54->temp1);
  }

  // Push to table(Append to the table)
  if (dt[array_num].handle) {
    if (config->cfg[array_num].sensor_type == TEROS11) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros11_col.vwc_avg_col, data_teros11->vwc);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros11_col.ta_avg_col,
                                  data_teros11->temperature);
    } else if (config->cfg[array_num].sensor_type == TEROS12) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros12_col.vwc_avg_col, data_teros12->vwc);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros12_col.ta_avg_col,
                                  data_teros12->temperature);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros12_col.ec_avg_col, data_teros12->ec);
    } else if (config->cfg[array_num].sensor_type == TEROS21) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros21_col.matricPotential_avg_col,
                                  data_teros21->matricPotential);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros21_col.temperature_avg_col,
                                  data_teros21->temperature);
    } else if (config->cfg[array_num].sensor_type == TEROS54) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.vwc1_col, data_teros54->vwc1);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.temp1_col, data_teros54->temp1);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.vwc2_col, data_teros54->vwc2);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.temp2_col, data_teros54->temp2);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.vwc3_col, data_teros54->vwc3);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.temp3_col, data_teros54->temp3);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.vwc4_col, data_teros54->vwc4);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].teros54_col.temp4_col, data_teros54->temp4);
    }

#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[%s] Pushed samples to data table", sensor_type);
    ESP_LOGI(TAG, "[%s] sampling_count: %d / max: %d", sensor_type, dt[array_num].handle->sampling_count,
             dt[array_num].handle->samples_maximum_size);

#endif
    // 데이터 처리 (평균, 필터링 등)
    datatable_process_samples(dt[array_num].handle);
    ESP_LOGI(TAG, "[%s] Processed samples in data table", sensor_type);

    // NOTE: 무조건 samples 데이터 처리해야 동작해. 아님. free 중복으로 에러남
    // 1분마다 samples 처리하도록 설정이 되어있음 data_table config에
    // 지금은 내부 internal time과 동기화가 되어 있지 않아서 수동으로 맞춰야 함.
    //

    /**
     * @brief Enforces periodic processing of sample data to prevent memory errors.
     *
     * Sample data must be consumed and cleared regularly; otherwise, duplicate `free()`
     * or memory corruption issues may occur. According to the data_table configuration,
     * samples are expected to be processed every 1 minute.
     *
     * Currently, this timing is not automatically synchronized with the internal time system,
     * so manual alignment is required to ensure sample processing occurs at the correct interval.
     *
     * Notes:
     * - Sample lifecycle must be managed carefully to avoid double-free or memory leaks.
     * - Timing should match the data_table's configuration (typically 60-second interval).
     * - Internal clock synchronization is planned but not implemented.
     */

    int retry_count = 0;
    while (dt[array_num].handle->sampling_count >= 3) {
      datatable_process_samples(dt[array_num].handle);
      // ESP_LOGI(TAG, "[%s] Processed samples in data table... count %d ", sensor_type, retry_count);
      retry_count += 1;
      vTaskDelay(pdMS_TO_TICKS(200));
      // vTaskDelay(pdMS_TO_TICKS(50));
    }

    /* serialize data-table and output in json format every 5-minutes (i.e. 12:00:00, 12:05:00, 12:10:00, etc.) */
    if (time_into_interval(dt[array_num].publish_interval.handle)) {
      // create root object for data-table
      cJSON* dt_json = cJSON_CreateObject();

      // convert the data-table to json object
      datatable_to_json(dt[array_num].handle, &dt_json);

      // render json data-table object to text and print
      char* dt_json_str = cJSON_Print(dt_json);
#ifdef SDI12_DEBUG_SET
      ESP_LOGI(TAG, "[%s] JSON Data-Table:\n%s", sensor_type, dt_json_str);
#endif
      memset(topic, 0x00, sizeof(topic));
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_SENSOR) == ESP_OK) {
        ret = publish_sensor_datatable(topic, dt_json_str);
        //  ESP_LOGI(TAG, "ret : %d", ret);
      }

      if (ret == -1) { /* Qos 0*/  // -1 fail,-2 box full
        char* parsed = parse_rows(dt_json_str);
        if (parsed != NULL) {
          memset(buf, 0x00, sizeof(buf));
          strncpy(buf, parsed, sizeof(buf) - 1);
          int len = strlen(buf);
          // TODO: Retry loop triggered when message sending fails
          //  Saved to internal flash when the network is disconnected and message sending fails
          //  append sensor_type string to the end of parsed buffer
          //  count,date,vwc,temperature,ec,sensor_type,port
          sprintf(buf + len, ",%s,%d", sensor_type, config->port);  // 여기는 실제 포트번호
          ESP_LOGI(TAG, "[FDATA] %s", buf);
          FDATA(BASE_PATH, "%s", buf);  // write to nvs data
        } else {
          ESP_LOGW(TAG, "Faild to parse JSON rows");
        }
      }

      // free-up json resources
      cJSON_free(dt_json_str);
      cJSON_Delete(dt_json);
    }

  } else {
    ESP_LOGW(TAG, "[%s] dt->handle is NULL, cannot push/process samples", sensor_type);
  }
  // 작업 완료 알림
  xEventGroupSetBits(done_group, TASK_DONE_BIT);
#ifdef SDI12_DEBUG_SET
  ESP_LOGI(TAG, "[%s] Task done, DONE_BIT set", sensor_type);
  ESP_LOGI(TAG, "[%s] Task completed", sensor_type);
#endif
}

static void handle_atmos_series(void* param) {
  int ret = 0;
  char buf[250] = { 0 };
  char topic[MAX_TOPIC_LEN] = { 0 };

  if (!param) {
    ESP_LOGE(TAG, "[ATMOS41] Invalid param (NULL)");
    return;
  }

  sensor_task_t* config = (sensor_task_t*)param;
  sensor_datatable_t* dt = config->dt;
  int array_num = config->port - 1;

  const char* sensor_type = sensor_type_to_str(config->cfg[array_num].sensor_type);
  // const char* sensor_type = sensor_type_to_str(TEROS12);
  if (strcmp(sensor_type, "UNKNOWN") == 0) {
    ESP_LOGW(TAG, "Unrecognized sensor type: %s", sensor_type);
  }
  ESP_LOGI(TAG, "[%s] Starting SDI-12 read...port[%d] ", sensor_type, config->port);

  weather_at41g2_data_t* data_at41g2 = NULL;
  weather_atmos22_data_t* data_atmos22 = NULL;

  if (config->cfg[array_num].sensor_type == ATMOS41) {
    data_at41g2 = sdi12_read_atmos41(array_num);  // 데이터 읽기
    if (!data_at41g2) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      goto next;
    }
  } else if (config->cfg[array_num].sensor_type == ATMOS22) {
    data_atmos22 = sdi12_read_atmos22(array_num);  // 데이터 읽기
    if (!data_atmos22) {
      ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
      goto next;
    }
  }

  // 데이터 테이블에 샘플 푸시
  if (dt[array_num].handle) {
    if (config->cfg[array_num].sensor_type == ATMOS41) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.solar_col, data_at41g2->solar);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.precipitation_col,
                                  data_at41g2->precipitation);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.strikes_col, data_at41g2->strikes);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.strikeDistance_col,
                                  data_at41g2->strikeDistance);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.windSpeed_col, data_at41g2->windSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.windDirection_col,
                                  data_at41g2->windDirection);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.gustWindSpeed_col,
                                  data_at41g2->gustWindSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.airTemperature_col,
                                  data_at41g2->airTemperature);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.vaporPressure_col,
                                  data_at41g2->vaporPressure);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.atmosphericPressure_col,
                                  data_at41g2->atmosphericPressure);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.relativeHumidity_col,
                                  data_at41g2->relativeHumidity);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.humiditySensorTemperature_col,
                                  data_at41g2->humiditySensorTemperature);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.xOrientation_col,
                                  data_at41g2->xOrientation);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.yOrientation_col,
                                  data_at41g2->yOrientation);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.nullValue_col, data_at41g2->nullValue);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.northWindSpeed_col,
                                  data_at41g2->northWindSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].at41g2_col.eastWindSpeed_col,
                                  data_at41g2->eastWindSpeed);
    } else if (config->cfg[array_num].sensor_type == ATMOS22) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.windSpeed_col,
                                  data_atmos22->windSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.windDirection_col,
                                  data_atmos22->windDirection);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.gustWindSpeed_col,
                                  data_atmos22->gustWindSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.airTemperature_col,
                                  data_atmos22->airTemperature);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.xOrientation_col,
                                  data_atmos22->xOrientation);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.yOrientation_col,
                                  data_atmos22->yOrientation);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.nullValue_col,
                                  data_atmos22->nullValue);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.northWindSpeed_col,
                                  data_atmos22->northWindSpeed);
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].atmos22_col.eastWindSpeed_col,
                                  data_atmos22->eastWindSpeed);
    }

#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[%s] Pushed samples to data table", sensor_type);
    ESP_LOGI(TAG, "[%s] sampling_count: %d / max: %d", sensor_type, dt[array_num].handle->sampling_count,
             dt[array_num].handle->samples_maximum_size);
#endif
    // 데이터 처리 (평균, 필터링 등)
    datatable_process_samples(dt[array_num].handle);
    ESP_LOGI(TAG, "[%s] Processed samples in data table", sensor_type);

    // NOTE: 무조건 samples 데이터 처리해야 동작해. 아님. free 중복으로 에러남
    // 1분마다 samples 처리하도록 설정이 되어있음 data_table config에
    // 지금은 내부 internal time과 동기화가 되어 있지 않아서 수동으로 맞춰야 함.

    /**
     * @brief Enforces periodic processing of sample data to prevent memory errors.
     *
     * Sample data must be consumed and cleared regularly; otherwise, duplicate `free()`
     * or memory corruption issues may occur. According to the data_table configuration,
     * samples are expected to be processed every 1 minute.
     *
     * Currently, this timing is not automatically synchronized with the internal time system,
     * so manual alignment is required to ensure sample processing occurs at the correct interval.
     *
     * Notes:
     * - Sample lifecycle must be managed carefully to avoid double-free or memory leaks.
     * - Timing should match the data_table's configuration (typically 60-second interval).
     * - Internal clock synchronization is planned but not implemented.
     */

    int retry_count = 0;
    while (dt[array_num].handle->sampling_count >= 3) {
      datatable_process_samples(dt[array_num].handle);
      // ESP_LOGI(TAG, "[%s] Processed samples in data table... count %d ", sensor_type, retry_count);
      retry_count += 1;
      vTaskDelay(pdMS_TO_TICKS(200));
      // vTaskDelay(pdMS_TO_TICKS(50));
    }

  next:
    /* serialize data-table and output in json format every 5-minutes (i.e. 12:00:00,
     * 12:05:00, 12:10:00, etc.) */
    if (time_into_interval(dt[array_num].publish_interval.handle)) {
      // create root object for data-table
      cJSON* dt_json = cJSON_CreateObject();

      // convert the data-table to json object

      datatable_to_json(dt[array_num].handle, &dt_json);

      // render json data-table object to text and print
      char* dt_json_str = cJSON_Print(dt_json);

#ifdef SDI12_DEBUG_SET
      ESP_LOGI(TAG, "[%s] JSON Data-Table:\n%s", sensor_type, dt_json_str);
#endif
      memset(topic, 0x00, sizeof(topic));
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_SENSOR) == ESP_OK) {
        ret = publish_sensor_datatable(topic, dt_json_str);
        // ESP_LOGI(TAG, "ret : %d", ret);
      }

      if (ret == -1) { /* Qos 0*/  // -1 fail,-2 box full
        char* parsed = parse_rows(dt_json_str);
        if (parsed != NULL) {
          memset(buf, 0x00, sizeof(buf));
          strncpy(buf, parsed, sizeof(buf) - 1);
          int len = strlen(buf);
          // TODO: Retry loop triggered when message sending fails
          //  Saved to internal flash when the network is disconnected and message sending
          //  fails append sensor_type string to the end of parsed buffer
          //  count,date,vwc,temperature,ec,sensor_type,port
          sprintf(buf + len, ",%s,%d", sensor_type, config->port);  // 여기는 실제 포트번호
          ESP_LOGI(TAG, "[FDATA] %s", buf);
          FDATA(BASE_PATH, "%s", buf);  // write to tb data
        } else {
          ESP_LOGW(TAG, "Faild to parse JSON rows");
        }
      }

      // free-up json resources
      cJSON_free(dt_json_str);
      cJSON_Delete(dt_json);
    }

  } else {
    ESP_LOGW(TAG, "[%s] dt->handle is NULL, cannot push/process samples", sensor_type);
  }
  // 작업 완료 알림
  xEventGroupSetBits(done_group, TASK_DONE_BIT);
#ifdef SDI12_DEBUG_SET
  ESP_LOGI(TAG, "[%s] Task done, DONE_BIT set", sensor_type);
  ESP_LOGI(TAG, "[%s] Task completed", sensor_type);
#endif
}

static void handle_apogee_series(void* param) {
  int ret = 0;
  char buf[250] = { 0 };
  char topic[MAX_TOPIC_LEN] = { 0 };

  if (!param) {
    ESP_LOGE(TAG, "[APOGEE_DATA] Invalid param (NULL)");
    return;
  }

  sensor_task_t* config = (sensor_task_t*)param;
  sensor_datatable_t* dt = config->dt;
  int array_num = config->port - 1;

  const char* sensor_type = sensor_type_to_str(config->cfg[array_num].sensor_type);

  if (strcmp(sensor_type, "UNKNOWN") == 0) {
    ESP_LOGW(TAG, "Unrecognized sensor type: %s", sensor_type);
  }
  ESP_LOGI(TAG, "[%s] Starting SDI-12 read...port[%d] ", sensor_type, config->port);

  apogee_sensor_t* apogee_sensor_data = NULL;

  if (config->cfg[array_num].sensor_type == APOGEE_S2_411) {
    apogee_sensor_data = sdi12_read_S2_411_412(array_num, APOGEE_S2_411);  // 데이터 읽기

  } else if (config->cfg[array_num].sensor_type == APOGEE_S2_412) {
    apogee_sensor_data = sdi12_read_S2_411_412(array_num, APOGEE_S2_412);  // 데이터 읽기

  } else if (config->cfg[array_num].sensor_type == APOGEE_SQ_521) {
    apogee_sensor_data = sdi12_read_SQ_521(array_num);  // 데이터 읽기

  } else {
    ESP_LOGE(TAG, "[%s] Unsupported Apogee sensor type", sensor_type);
    goto next;
  }

  if (!apogee_sensor_data) {
    ESP_LOGE(TAG, "[%s] Failed to read data from SDI-12 sensor", sensor_type);
    goto next;
  }

  // push in to data-table
  if (dt[array_num].handle) {
    if (config->cfg[array_num].sensor_type == APOGEE_S2_411) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].apogee_data_col.NDVI_UP_WARD,
                                  apogee_sensor_data->NDVI_UP_WARD);
    } else if (config->cfg[array_num].sensor_type == APOGEE_S2_412) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].apogee_data_col.NDVI_DOWN_WARD,
                                  apogee_sensor_data->NDVI_DOWN_WARD);
    } else if (config->cfg[array_num].sensor_type == APOGEE_SQ_521) {
      datatable_push_float_sample(dt[array_num].handle, dt[array_num].apogee_data_col.PPFD, apogee_sensor_data->PPFD);
    }

#ifdef SDI12_DEBUG_SET
    ESP_LOGI(TAG, "[%s] Pushed samples to data table", sensor_type);
    ESP_LOGI(TAG, "[%s] sampling_count: %d / max: %d", sensor_type, dt[array_num].handle->sampling_count,
             dt[array_num].handle->samples_maximum_size);
#endif
    // 데이터 처리 (평균, 필터링 등)
    datatable_process_samples(dt[array_num].handle);
    ESP_LOGI(TAG, "[%s] Processed samples in data table", sensor_type);

    // NOTE: 무조건 samples 데이터 처리해야 동작해. 아님. free 중복으로 에러남
    // 1분마다 samples 처리하도록 설정이 되어있음 data_table config에
    // 지금은 내부 internal time과 동기화가 되어 있지 않아서 수동으로 맞춰야 함.

    int retry_count = 0;
    while (dt[array_num].handle->sampling_count >= 3) {
      datatable_process_samples(dt[array_num].handle);
      // ESP_LOGI(TAG, "[%s] Processed samples in data table... count %d ", sensor_type, retry_count);
      retry_count += 1;
      vTaskDelay(pdMS_TO_TICKS(200));
      // vTaskDelay(pdMS_TO_TICKS(50));
    }

  next:
    /* serialize data-table and output in json format every 5-minutes (i.e. 12:00:00,
     * 12:05:00, 12:10:00, etc.) */
    if (time_into_interval(dt[array_num].publish_interval.handle)) {
      // create root object for data-table
      cJSON* dt_json = cJSON_CreateObject();

      // convert the data-table to json object

      datatable_to_json(dt[array_num].handle, &dt_json);

      // render json data-table object to text and print
      char* dt_json_str = cJSON_Print(dt_json);
#ifdef SDI12_DEBUG_SET
      ESP_LOGI(TAG, "[%s] JSON Data-Table:\n%s", sensor_type, dt_json_str);
#endif
      memset(topic, 0x00, sizeof(topic));
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_SENSOR) == ESP_OK) {
        ret = publish_sensor_datatable(topic, dt_json_str);
        // ESP_LOGI(TAG, "ret : %d", ret);
      }

      if (ret == -1) { /* Qos 0*/  // -1 fail,-2 box full
        char* parsed = parse_rows(dt_json_str);
        if (parsed != NULL) {
          memset(buf, 0x00, sizeof(buf));
          strncpy(buf, parsed, sizeof(buf) - 1);
          int len = strlen(buf);
          // TODO: Retry loop triggered when message sending fails
          //  Saved to internal flash when the network is disconnected and message sending
          //  fails append sensor_type string to the end of parsed buffer
          //  count,date,vwc,temperature,ec,sensor_type,port
          sprintf(buf + len, ",%s,%d", sensor_type, config->port);  // 여기는 실제 포트번호
          ESP_LOGI(TAG, "[FDATA] %s", buf);
          FDATA(BASE_PATH, "%s", buf);  // write to tb data
        } else {
          ESP_LOGW(TAG, "Faild to parse JSON rows");
        }
      }

      // free-up json resources
      cJSON_free(dt_json_str);
      cJSON_Delete(dt_json);
    }

  } else {
    ESP_LOGW(TAG, "[%s] dt->handle is NULL, cannot push/process samples", sensor_type);
  }
  // 작업 완료 알림
  xEventGroupSetBits(done_group, TASK_DONE_BIT);

#ifdef SDI12_DEBUG_SET
  ESP_LOGI(TAG, "[%s] Task done, DONE_BIT set", sensor_type);
  ESP_LOGI(TAG, "[%s] Task completed", sensor_type);
#endif
}

// =============================
// Task Entry Function
// =============================

/**
 * @brief Sensor task loop that waits for trigger and port detection events.
 *
 * Each sensor task waits for a specific trigger bit from a shared event group (trigger_group).
 * Once triggered, it checks the corresponding sensor port's detection bit from the AutoDetect manager's
 * event group (sensor_event_group). If the port is active (bit is set), the task invokes the registered
 * action callback.
 *
 * This design allows each task to:
 * - Be triggered externally (e.g., from FSM or AutoDetect controller)
 * - Wait for a valid sensor connection state on its assigned port
 * - Execute a port-specific action when ready
 *
 * Notes:
 * - During initialization, all sensor port bits (0–5) are set to simulate connected state
 * - The task runs indefinitely, handling one trigger-action cycle per loop
 * - Uses FreeRTOS event groups for synchronization and signaling
 */

static sensor_task_t sensor_tasks[MAX_SENSOR_TASKS];
static int task_count = 0;

void sensor_set_all_ports_connected(void) {
  sensor_ad_manager_t* mgr = sensor_ad_get_instance();
  if (!mgr || !mgr->sensor_event_group) {
    ESP_LOGE(TAG, "sensor_event_group not initialized");
    return;
  }
  // Set bits for port 0 to 5 → 0x3F = BIT0 | BIT1 | ... | BIT5
  xEventGroupSetBits(mgr->sensor_event_group, 0x3F);
  ESP_LOGI(TAG, "Initialized all sensor event bits (ports 0~5) to connected.");
}

static void sensor_task_entry(void* pvParameters) {
  sensor_task_t* config = (sensor_task_t*)pvParameters;
  ESP_LOGI(TAG, "[%s] Task started. Waiting on bit 0x%02X...", config->task_name, (unsigned int)config->bit);

  sensor_ad_manager_t* mgr = sensor_ad_get_instance();
  // AutoDetect Event bit 초기화: 모든 포트를 연결된 상태로 설정 (PORT 0~5)
  for (int i = 0; i < 6; ++i) {
    xEventGroupSetBits(mgr->sensor_event_group, SENSOR_PORT_EVENT_BIT(i));
  }

  while (true) {
    // Bit used to trigger the strategy task (set by FSM as an event signal)
    ESP_LOGW(TAG, "[FSM -> Task] Waiting started: PORT[%d]_%s, BitMask: 0x%04X", config->port, config->task_name,
             SENSOR_PORT_EVENT_BIT((unsigned int)config->port - 1));

    xEventGroupWaitBits(config->trigger_group, config->bit,
                        pdTRUE,   // clear on exit
                        pdFALSE,  // wait any
                        portMAX_DELAY);

    EventBits_t wait_mask = SENSOR_PORT_EVENT_BIT(config->port - 1);

    /**
     * @brief Wait for sensor port detection before proceeding to avoid task conflict.
     *
     * This section ensures that the task only proceeds when its assigned sensor port
     * is marked as connected (bit is set) by the AutoDetect system. By waiting for
     * the corresponding bit in the sensor_event_group, this logic prevents the task
     * from executing prematurely while AutoDetect is still probing or another task
     * is using the same port.
     *
     * Purpose:
     * - Prevent sensor task conflict or race conditions during sensor detection.
     * - Synchronize sensor task execution with AutoDetect status.
     * - Ensure the port is ready before reading actual sensor data.
     *
     * Notes:
     * - Wait is blocking (portMAX_DELAY) until the event bit is set externally.
     * - Bits are NOT cleared after wait; they must be cleared manually by the controller.
     */

    EventBits_t current_bits = xEventGroupGetBits(mgr->sensor_event_group);
    ESP_LOGI(TAG, "[AutoDetect -> Task] Current bits: 0x%04X, Waiting for: 0x%04X", (unsigned int)current_bits,
             (unsigned int)wait_mask);

    // AutoDetect wait event bit group
    EventBits_t result_bits = xEventGroupWaitBits(mgr->sensor_event_group, wait_mask,
                                                  pdFALSE,  // 대기 후 클리어하지 않음 (수동 클리어 필요)
                                                  pdFALSE,  // 하나라도 Set되면 대기 해제
                                                  portMAX_DELAY);

    // Checking Event group bit
    ESP_LOGW(TAG, "[AutoDetect] Wake-up triggered: PORT[%d]_%s, ResultBits: 0x%04X", config->port, config->task_name,
             (unsigned int)result_bits);

    if (config->action) {
      config->action(config);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================
// Public Functions
// =============================

/**
 * @brief Pass the sensor configuration and data table parameters to each task
 * so that they operate based on their assigned port.
 */
bool strategy_create_tasks(void) {
  ESP_LOGI(TAG, "Creating %d strategy tasks", strategy_entry_count);
  for (int i = 0; i < strategy_entry_count; ++i) {
    const sensor_strategy_entry_t* entry = &sensor_strategies[i];

    if (task_count >= MAX_SENSOR_TASKS) {
      ESP_LOGE(TAG, "Too many sensor tasks!");
      return false;
    }

    sensor_task_t* task = &sensor_tasks[task_count];
    task->trigger_group = xEventGroupCreate();
    task->bit = entry->trigger_bit;
    task->action = entry->action;
    task->task_name = entry->task_name;
    task->dt = dt;
    task->cfg = cfg;
    task->port = entry->port;
    BaseType_t result = xTaskCreatePinnedToCore(sensor_task_entry, entry->task_name, 4096, task, 5, &task->handle, 1);

    if (result != pdPASS) {
      ESP_LOGE(TAG, "Failed to create task: %s", entry->task_name);
      return false;
    }

    ++task_count;
    ESP_LOGI(TAG, "Task created: %s", entry->task_name);
  }
  return true;
}

void sensor_strategy_trigger(sdi12_sensor_type_t type, bool status) {
  if (type >= SENSOR_TYPE_COUNT) {
    ESP_LOGW(TAG, "Invalid sensor type");
    return;
  }

  const EventBits_t bit = sensor_strategies[type].trigger_bit;

  for (int i = 0; i < task_count; ++i) {
    if (sensor_tasks[i].bit == bit) {
      if (status)
        xEventGroupSetBits(sensor_tasks[i].trigger_group, bit);
      else
        xEventGroupClearBits(sensor_tasks[i].trigger_group, bit);
      return;
    }
  }
  ESP_LOGW(TAG, "No task found for sensor type %d", type);
}

void strategy_task_mgr_sensor_start(void) {
  cfg = sensor_cfg_instance();
  dt = sensor_dt_instance();
  sensor_strategy_entry_t* ptr = sensor_strategies;
  strategy_entry_count = 0;

  if (!cfg) {
    ESP_LOGE(TAG, "sensor_cfg_instance() returned NULL");
    return;
  }

  if (!dt) {
    ESP_LOGE(TAG, "sensor_dt_instance() returned NULL");
    return;
  }

  for (int i = 0; i < SENSOR_PORT_COUNT; i++) {
    if (cfg[i].current_state == SENSOR_PORT_STATUS_READY && dt[i].status == 1) {
      sensor_strategy_entry_t* entry = &ptr[strategy_entry_count];  // <== 여기 핵심
      entry->trigger_bit = BIT(i);

      switch (cfg[i].sensor_type) {
        case TEROS11:
          entry->type = TEROS11;
          entry->action = handle_teros_series;
          entry->task_name = sensor_type_to_str(TEROS11);
          entry->port = cfg[i].port;
          break;

        case TEROS12:
          entry->type = TEROS12;
          entry->action = handle_teros_series;
          entry->task_name = sensor_type_to_str(TEROS12);
          entry->port = cfg[i].port;
          break;

        case TEROS21:
          entry->type = TEROS21;
          entry->action = handle_teros_series;
          entry->task_name = sensor_type_to_str(TEROS21);
          entry->port = cfg[i].port;
          break;

        case TEROS54:
          entry->type = TEROS54;
          entry->action = handle_teros_series;
          entry->task_name = sensor_type_to_str(TEROS54);
          entry->port = cfg[i].port;
          break;

        case ATMOS22:
          entry->type = ATMOS22;
          entry->action = handle_atmos_series;
          entry->task_name = sensor_type_to_str(ATMOS22);
          entry->port = cfg[i].port;
          break;

        case ATMOS41:
          entry->type = ATMOS41;
          entry->action = handle_atmos_series;
          entry->task_name = sensor_type_to_str(ATMOS41);
          entry->port = cfg[i].port;
          break;

        case APOGEE_S2_411:
          entry->type = APOGEE_S2_411;
          entry->action = handle_apogee_series;
          entry->task_name = sensor_type_to_str(APOGEE_S2_411);
          entry->port = cfg[i].port;
          break;

        case APOGEE_S2_412:
          entry->type = APOGEE_S2_412;
          entry->action = handle_apogee_series;
          entry->task_name = sensor_type_to_str(APOGEE_S2_412);
          entry->port = cfg[i].port;
          break;

        case APOGEE_SQ_521:
          entry->type = APOGEE_SQ_521;
          entry->action = handle_apogee_series;
          entry->task_name = sensor_type_to_str(APOGEE_SQ_521);
          entry->port = cfg[i].port;
          break;

        default: ESP_LOGW(TAG, "Unsupported sensor type at port %d", i); continue;  // 잘못된 경우 건너뛰기
      }

      ESP_LOGI(TAG, "cfg[%d]  Set_port[%d] Set_type[%d] (cfg.state=%d, dt.status=%d)", i, cfg[i].port,
               cfg[i].sensor_type, cfg[i].current_state, dt[i].status);

      ++strategy_entry_count;

    } else {
      ESP_LOGI(TAG, "cfg[%d] skipped (state=%d, dt.status=%d)", i, cfg[i].current_state, dt[i].status);
    }
  }

  if (!strategy_create_tasks()) {
    ESP_LOGE("MAIN", "Failed to create sensor tasks");
    return;
  }
}

/**
 * @brief Finite State Machine (FSM) task loop to coordinate sensor sampling triggers.
 *
 * This FSM coordinates the triggering of multiple sensor tasks in sequence
 * to ensure sufficient samples are collected for data-table mapping.
 *
 * - The data-table (DT) needs 3 samples per minute (mapping 3 times in 1 minute).
 * - To guarantee at least 3 valid samples even with potential delays,
 *   the FSM triggers 4 cycles per minute.
 * - Each sensor task is triggered in order; the FSM waits for task completion (TASK_DONE_BIT).
 * - If a task takes too long (timeout), the FSM increments a timeout counter.
 * - If timeouts exceed MAX_TIME_OUT_COUNT, the system reboots via esp_restart().
 * - Between cycles, the FSM ensures a fixed delay (trigger_interval) before restarting.
 *
 * Key behavior:
 * - Sequentially triggers sensor tasks and waits for completion.
 * - Manages delays to align with data-table sampling requirements.
 * - Protects against indefinite blocking via timeout & restart mechanism.
 *
 * States:
 * - TRIGGER_STATE_INIT: Reset index, mark start time.
 * - TRIGGER_STATE_TRIGGER_ONE: Trigger next sensor task.
 * - TRIGGER_STATE_WAIT_ONE: Wait for task to signal done or timeout.
 * - TRIGGER_STATE_DONE: All tasks in this cycle done.
 * - TRIGGER_STATE_WAIT: Wait for next cycle interval before restarting.
 *
 * Notes:
 * - Avoids sample loss due to delays by over-triggering (4 triggers for 3 needed).
 * - Sequential triggering prevents simultaneous task congestion.
 * - Timeout guard prevents system hang.
 */

/* clang-format off */

 #define MAX_TIME_OUT_COUNT 6
typedef enum { 
  TRIGGER_STATE_INIT, 
  TRIGGER_STATE_WAIT, 
  TRIGGER_STATE_WAIT_ONE,
  TRIGGER_STATE_TRIGGER_ONE, 
  TRIGGER_STATE_DONE 
} trigger_state_t;

typedef struct {
  int current_index;
  TickType_t last_trigger_time;
  trigger_state_t state;
} trigger_fsm_t;

static void fsm_task_loop(void* pvParameters) {

  uint8_t time_out_count = 0;
  TickType_t task_start_time = 0;
  trigger_fsm_t fsm = { 
    .current_index = 0, 
    .last_trigger_time = xTaskGetTickCount(), 
    .state = TRIGGER_STATE_INIT
  };
  /* clang-format on */
  const TickType_t trigger_interval = pdMS_TO_TICKS(15000);    // 전체 트리거 주기
  const TickType_t inter_task_timeout = pdMS_TO_TICKS(30000);  // 태스크 실행 최대 대기시간

  done_group = xEventGroupCreate();  ///< FSM waits on this group for task completion (e.g. DONE_BIT)

  if (done_group) {
    ESP_LOGI(TAG, "done_group created successfully.");
  } else {
    ESP_LOGE(TAG, "Failed to create done_group.");
  }

  while (true) {
    switch (fsm.state) {
      case TRIGGER_STATE_INIT:
        fsm.current_index = 0;
        fsm.state = TRIGGER_STATE_TRIGGER_ONE;
        fsm.last_trigger_time = xTaskGetTickCount();  // 시작시점을 바로 넣는다
        break;

      case TRIGGER_STATE_TRIGGER_ONE:

        if (fsm.current_index < task_count) {
          sensor_task_t* task = &sensor_tasks[fsm.current_index];

          xEventGroupClearBits(done_group, TASK_DONE_BIT);
          task_start_time = xTaskGetTickCount();
          xEventGroupSetBits(task->trigger_group, task->bit);
          ESP_LOGI(TAG, "[FSM] Triggered task: %s", task->task_name);

          // 다음 상태로 전환
          fsm.state = TRIGGER_STATE_WAIT_ONE;
        } else {
          fsm.state = TRIGGER_STATE_DONE;
        }

        break;
      case TRIGGER_STATE_WAIT_ONE: {
        sensor_task_t* task = &sensor_tasks[fsm.current_index];
        EventBits_t bits = xEventGroupWaitBits(done_group, TASK_DONE_BIT,
                                               pdTRUE,  // 자동 클리어
                                               pdFALSE, inter_task_timeout);

        TickType_t task_end_time = xTaskGetTickCount();
        TickType_t duration_ms = (task_end_time - task_start_time) * portTICK_PERIOD_MS;

        if (bits & TASK_DONE_BIT) {
          ESP_LOGW(TAG, "[FSM] Task %s completed in %lu ms", task->task_name, (unsigned long)duration_ms);
        } else {
          ESP_LOGW(TAG, "[FSM] Task %s timeout after %lu ms", task->task_name, (unsigned long)duration_ms);
          // NOTE: if timeout occur continuously, reboot
          time_out_count++;
          ESP_LOGE(TAG, "current_count [%d]", time_out_count);
          if (time_out_count == MAX_TIME_OUT_COUNT) {
            esp_restart();
          }
        }

        fsm.current_index++;
        fsm.state = TRIGGER_STATE_TRIGGER_ONE;

        break;
      }

      case TRIGGER_STATE_DONE:
        // fsm.last_trigger_time = xTaskGetTickCount();
        fsm.state = TRIGGER_STATE_WAIT;
        break;

      case TRIGGER_STATE_WAIT:
        if (xTaskGetTickCount() - fsm.last_trigger_time >= trigger_interval) {
          fsm.state = TRIGGER_STATE_INIT;
        } else {
          vTaskDelay(pdMS_TO_TICKS(100));  // Idle delay
        }
        break;
    }
  }
}

/**
 * @brief Starts the trigger FSM task for controlling sensor strategy execution.
 *
 * This function creates a FreeRTOS task pinned to core 1 that runs the FSM
 * responsible for triggering sensor strategy tasks based on conditions.
 * Logs the result and stores the task handle locally for debugging or future use.
 */
void strategy_trigger_task_start(void) {
  static TaskHandle_t trigger_task_handle = NULL;

  BaseType_t result = xTaskCreatePinnedToCore(fsm_task_loop,         // Task function
                                              "TriggerTask",         // Name
                                              4096,                  // Stack size
                                              NULL,                  // Parameters
                                              5,                     // Priority
                                              &trigger_task_handle,  // Task handle
                                              1                      // Core ID
  );

  if (result == pdPASS) {
    ESP_LOGI(TAG, "[TriggerTask] Task created. Handle = %p", trigger_task_handle);
  } else {
    ESP_LOGE(TAG, "[TriggerTask] Failed to create task.");
  }
}
