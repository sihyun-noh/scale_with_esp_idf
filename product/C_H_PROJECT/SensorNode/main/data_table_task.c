#include <string.h>
#include "esp_err.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "sdi12_task.h"
#include "stdio.h"
#include "esp_log.h"
#include "mqtt_publish.h"
#include "mqtt_config.h"
#include "file_log.h"
#include "littlefs_impl.h"
#include "sensor_cfg_config.h"
#include "data_table_task.h"

#define PORT_DT_HANDLER(i) "port_dt_handler" #i

static const char* TAG = "[dataTable_task]";

#if 0
void callback_fc(void* handle, datatable_event_t msg) {
  if (handle != NULL) {
    ESP_LOGI(TAG, "dt event type %d _ %s", msg.type, msg.message);
  }
}

static const datatable_event dt_1min_hdl_event = callback_fc;
#endif

static sensor_datatable_t port_tables[SENSOR_PORT_COUNT];

// singleton instance
sensor_datatable_t* sensor_dt_instance(void) {
  return port_tables;
}

// 매핑된 센서 config을 불러
bool init_datatable_for_port(int port_index, const sensor_port_cfg_t* cfg, sensor_datatable_t* dt) {
  if (cfg->current_state != SENSOR_PORT_STATUS_READY) {
    ESP_LOGI(TAG, "Port %d disabled, skip datatable init", cfg->port);
    dt->status = 0;  // 1: Active, 0: deactivate
    return false;
  }
  // dt status active
  // This check is performed in the strategy_task
  dt->status = 1;

  /* clang-format off */
  time_into_interval_config_t pub_cfg = {
    .name = "interval",  // 아래에서 버퍼를 할당·포맷팅
    .interval_type = TIME_INTO_INTERVAL_SEC, 
    .interval_period = cfg->server_config.publish_interval * 60, 
    .interval_offset = 10
  };

  datatable_config_t dt_cfg = {
    .name = NULL,  // 아래에서 버퍼를 할당·포맷팅
    .columns_size = cfg->columns_size,
    .rows_size    = cfg->rows_size,
    .data_storage_type = DATATABLE_DATA_STORAGE_MEMORY_RING,
    //.data_storage_type = DATATABLE_DATA_STORAGE_MEMORY_RESET,
    .sampling_config = {
      .name          = NULL,
      .interval_type = TIME_INTO_INTERVAL_SEC,
//      .interval_period = cfg->sampling_period_sec,
     // .interval_period = 20, // static 20초1번씩 
      .interval_period = 50, // static 60초1번씩 -> 기상대
      .interval_offset = 0,
    },
    .processing_config = {
      .name          = NULL,
      .interval_type = TIME_INTO_INTERVAL_MIN,
//      .interval_period = cfg->processing_interval_period_min,
      .interval_period = 1,
      .interval_offset = 0,
    },
   // .event_handler = cfg->event_handler,
  };
  /* clang-format on */

  char name_buf[32];
  const char* type_name = sensor_type_to_str(cfg->sensor_type);
  snprintf(name_buf, sizeof(name_buf), "%dP_%dm_%s", cfg->port, cfg->server_config.publish_interval, type_name);
  dt_cfg.name = strdup(name_buf);

  char samp_buf[32];
  snprintf(samp_buf, sizeof(samp_buf), "%dP_smp", cfg->port);
  dt_cfg.sampling_config.name = strdup(samp_buf);

  char proc_buf[32];
  snprintf(proc_buf, sizeof(proc_buf), "%dm_proc", 1);  // static 1 min
  dt_cfg.processing_config.name = strdup(proc_buf);

  // data table 실제 초기화 호출
  if (datatable_init(&dt_cfg, &dt->handle) != ESP_OK) {
    ESP_LOGE(TAG, "Port %d datatable_init failed", cfg->port);
    ESP_LOGE(TAG, "table name : %s ", dt_cfg.name);
    return false;
  }

  if (time_into_interval_init(&pub_cfg, &dt->publish_interval.handle) != ESP_OK) {
    ESP_LOGE(TAG, "time_into_interval_new, new time-into-interval handle failed");
    return false;
  }

  ESP_LOGI(TAG, "Port %d datatable initialized (handle=%p)", cfg->port, dt->handle);

  switch (cfg->sensor_type) {
    /* TEROS TYPE */
    case TEROS11:
      datatable_add_float_smp_column(dt->handle, "vwc", &dt->teros11_col.vwc_avg_col);
      datatable_add_float_smp_column(dt->handle, "temp", &dt->teros11_col.ta_avg_col);
      break;
    case TEROS12:
      datatable_add_float_smp_column(dt->handle, "vwc", &dt->teros12_col.vwc_avg_col);
      datatable_add_float_smp_column(dt->handle, "temp", &dt->teros12_col.ta_avg_col);
      datatable_add_float_smp_column(dt->handle, "ec", &dt->teros12_col.ec_avg_col);
      break;
      // case TEROS14:
    case TEROS21:
      datatable_add_float_smp_column(dt->handle, "matricPotential", &dt->teros21_col.matricPotential_avg_col);
      datatable_add_float_smp_column(dt->handle, "temperature", &dt->teros21_col.temperature_avg_col);
      break;

    case TEROS54:
      datatable_add_float_smp_column(dt->handle, "vwc1", &dt->teros54_col.vwc1_col);
      datatable_add_float_smp_column(dt->handle, "temperature1", &dt->teros54_col.temp1_col);
      datatable_add_float_smp_column(dt->handle, "vwc2", &dt->teros54_col.vwc2_col);
      datatable_add_float_smp_column(dt->handle, "temperature2", &dt->teros54_col.temp2_col);
      datatable_add_float_smp_column(dt->handle, "vwc3", &dt->teros54_col.vwc3_col);
      datatable_add_float_smp_column(dt->handle, "temperature3", &dt->teros54_col.temp3_col);
      datatable_add_float_smp_column(dt->handle, "vwc4", &dt->teros54_col.vwc4_col);
      datatable_add_float_smp_column(dt->handle, "temperature4", &dt->teros54_col.temp4_col);
      break;

    /* ATMOS TYPE */

    /* case ATMOS21: */
    case ATMOS22:
      datatable_add_float_smp_column(dt->handle, "windSpeed", &dt->atmos22_col.windSpeed_col);
      datatable_add_float_smp_column(dt->handle, "windDirection", &dt->atmos22_col.windDirection_col);
      datatable_add_float_smp_column(dt->handle, "gustWindSpeed", &dt->atmos22_col.gustWindSpeed_col);
      datatable_add_float_smp_column(dt->handle, "airTemperature", &dt->atmos22_col.airTemperature_col);
      datatable_add_float_smp_column(dt->handle, "xOrientation", &dt->atmos22_col.xOrientation_col);
      datatable_add_float_smp_column(dt->handle, "yOrientation", &dt->atmos22_col.yOrientation_col);
      datatable_add_float_smp_column(dt->handle, "nullValue", &dt->atmos22_col.nullValue_col);
      datatable_add_float_smp_column(dt->handle, "northWindSpeed", &dt->atmos22_col.northWindSpeed_col);
      datatable_add_float_smp_column(dt->handle, "eastWindSpeed", &dt->atmos22_col.eastWindSpeed_col);
      break;

    /* case ATMOS31: */
    case ATMOS41:
      datatable_add_float_smp_column(dt->handle, "solar", &dt->at41g2_col.solar_col);
      datatable_add_float_smp_column(dt->handle, "precipitation", &dt->at41g2_col.precipitation_col);
      datatable_add_float_smp_column(dt->handle, "strikes", &dt->at41g2_col.strikes_col);
      datatable_add_float_smp_column(dt->handle, "strikeDistance", &dt->at41g2_col.strikeDistance_col);
      datatable_add_float_smp_column(dt->handle, "windSpeed", &dt->at41g2_col.windSpeed_col);
      datatable_add_float_smp_column(dt->handle, "windDirection", &dt->at41g2_col.windDirection_col);
      datatable_add_float_smp_column(dt->handle, "gustWindSpeed", &dt->at41g2_col.gustWindSpeed_col);
      datatable_add_float_smp_column(dt->handle, "airTemperature", &dt->at41g2_col.airTemperature_col);
      datatable_add_float_smp_column(dt->handle, "vaporPressure", &dt->at41g2_col.vaporPressure_col);
      datatable_add_float_smp_column(dt->handle, "phericPressure", &dt->at41g2_col.atmosphericPressure_col);
      datatable_add_float_smp_column(dt->handle, "relativeHumi", &dt->at41g2_col.relativeHumidity_col);
      datatable_add_float_smp_column(dt->handle, "humiSensorTemp", &dt->at41g2_col.humiditySensorTemperature_col);
      datatable_add_float_smp_column(dt->handle, "xOrientation", &dt->at41g2_col.xOrientation_col);
      datatable_add_float_smp_column(dt->handle, "yOrientation", &dt->at41g2_col.yOrientation_col);
      datatable_add_float_smp_column(dt->handle, "nullValue", &dt->at41g2_col.nullValue_col);
      datatable_add_float_smp_column(dt->handle, "northWindSpeed", &dt->at41g2_col.northWindSpeed_col);
      datatable_add_float_smp_column(dt->handle, "eastWindSpeed", &dt->at41g2_col.eastWindSpeed_col);
      break;

    /* APOGEE TYPE */
    case APOGEE_S2_412:
      datatable_add_float_smp_column(dt->handle, "NDVI", &dt->apogee_data_col.NDVI);
      break;
      // case APOGEE_SP_421:
    case APOGEE_SQ_521:
      datatable_add_float_smp_column(dt->handle, "PPFD", &dt->apogee_data_col.PPFD);
      break;
      // case APOGEE_SU_221: datatable_add_float_avg_column(dt->handle, "Radiation", &dt->radiation_col); break;

    default: ESP_LOGW(TAG, "Port %d: Unknown sensor type %d, no columns added", cfg->port, cfg->sensor_type); break;
  }

  ESP_LOGI(TAG, "Port %d columns: Pa@%u, Ta_avg@%u, Ta_min@%u, Ta_max@%u, Td@%u", cfg->port, dt->pa_avg_col,
           dt->ta_avg_col, dt->ta_min_col, dt->ta_max_col, dt->td_avg_col);

  return true;
}

esp_err_t data_table_init_all(void) {
  sensor_port_cfg_t* cfg = sensor_cfg_instance();
  if (!cfg) {
    ESP_LOGE(TAG, "sensor_cfg_instance() returned NULL");
    return ESP_FAIL;
  }
  for (int i = 0; i < SENSOR_PORT_COUNT; i++) {
    // port_tables 는 sensor_datatable_t 배열
    ESP_LOGW(TAG, "cfg[%d]  Set_port[%d] Set_type[%d] (cfg.state=%d), columns_size[%d]", i, cfg[i].port,
             cfg[i].sensor_type, cfg[i].current_state, cfg[i].columns_size);
    if (!init_datatable_for_port(i, &cfg[i], &port_tables[i])) {
      ESP_LOGE(TAG, "Table initialized fail![port : %d]", i + 1);

      // return ESP_FAIL;
    }
  }
  return ESP_OK;
}

/* clang-format off */
static datatable_handle_t       dt_1min_hdl;          /*  data-table handle */
static datatable_config_t       dt_1min_cfg = {       /*  data-table configuration */
    .name                       = "1min_tbl",
    .columns_size               = 10,
    .rows_size                  = 1,
    .data_storage_type          = DATATABLE_DATA_STORAGE_MEMORY_RING,
    .sampling_config            = {
        .name                   = "10s_smp",
        .interval_type          = TIME_INTO_INTERVAL_SEC,
        .interval_period        = 10,
        .interval_offset        = 0
    },
    .processing_config          = {
        .name                   = "1m_proc",
        .interval_type          = TIME_INTO_INTERVAL_MIN,
        .interval_period        = 1,
        .interval_offset        = 0
    },
 // .event_handler = dt_1min_hdl_event,
};
static uint8_t              dt_1min_pa_avg_col_index;     /* data-table average atmospheric pressure (pa-avg) column index reference */
static uint8_t              dt_1min_ta_avg_col_index;     /* data-table average air temperature (ta-avg) column index reference */
static uint8_t              dt_1min_ta_max_col_index;     /* data-table minimum air temperature (ta-min) column index reference */
static uint8_t              dt_1min_ta_min_col_index;     /* data-table maximum air temperature (ta-max) column index reference */
static uint8_t              dt_1min_td_avg_col_index;     /* data-table average dew-point temperature (td-avg) column index reference */

/* clang-format on */

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

esp_err_t data_table_init() {
  return data_table_init_all();

#if 0
  // create a new data-table handle for the type 1
  datatable_init(&dt_1min_cfg, &dt_1min_hdl);
  if (dt_1min_hdl == NULL) {
    ESP_LOGE(TAG, "datatable_new, new data-table handle failed");
    abort();
  }

  // configure data-table columns

  // add float average column to data-table
  datatable_add_float_avg_column(dt_1min_hdl, "Pa_1-Avg",
                                 &dt_1min_pa_avg_col_index);  // column index 2
                                                              // add float average column to data-table
  datatable_add_float_avg_column(dt_1min_hdl, "Ta_1-Avg",
                                 &dt_1min_ta_avg_col_index);  // column index 3
  datatable_add_float_avg_column(dt_1min_hdl, "Ta_1-Avg",
                                 &dt_1min_ta_avg_col_index);  // column index 3
  datatable_add_float_avg_column(dt_1min_hdl, "Ta_1-Avg",
                                 &dt_1min_ta_avg_col_index);  // column index 3
                                                              // add float minimum column to data-table
  datatable_add_float_min_column(dt_1min_hdl, "Ta_1-Min", &dt_1min_ta_min_col_index);  // column index 4
  // add float maximum column to data-table
  datatable_add_float_max_column(dt_1min_hdl, "Ta_1-Max", &dt_1min_ta_max_col_index);  // column index 5
  // add float average column to data-table
  datatable_add_float_avg_column(dt_1min_hdl, "Td_1-Avg", &dt_1min_td_avg_col_index);  // column index 6
#endif
}

void dt_1min_smp_task(void* pvParameters) {
  /* clang-format off */
  int ret=0;
  char buf[250] = { 0 };
  char topic[MAX_TOPIC_LEN] ={ 0 };

  time_into_interval_handle_t dt_1min_tii_5min_hdl;
  time_into_interval_config_t dt_1min_tii_5min_cfg = {
        .name               = "5min_interval",
        .interval_type      = TIME_INTO_INTERVAL_SEC,
        .interval_period    = 1 * 60,
        .interval_offset    = 10
    };

  // create a new time-into-interval handle - task system clock synchronization
    time_into_interval_init(&dt_1min_tii_5min_cfg, &dt_1min_tii_5min_hdl);
    if (dt_1min_tii_5min_hdl == NULL) ESP_LOGE(TAG, "time_into_interval_new, new time-into-interval handle failed");

  /* clang-format on */

  for (;;) {
    /* delay data-table sampling task until sampling interval has elapsed */
    datatable_sampling_task_delay(dt_1min_hdl);

    /* measure samples from sensors and set sensor variables (pa, ta, td)  */

    uint8_t portId = 1;
    teros12_data_t* tero12 = sdi12_read_teros12(portId);

    // push samples onto the data buffer stack for processing
    datatable_push_float_sample(dt_1min_hdl, dt_1min_pa_avg_col_index, tero12->vwc);
    datatable_push_float_sample(dt_1min_hdl, dt_1min_ta_avg_col_index, tero12->temperature);
    datatable_push_float_sample(dt_1min_hdl, dt_1min_ta_min_col_index, tero12->ec);

    // process data buffer stack samples (i.e. data-table's configured processing interval)
    datatable_process_samples(dt_1min_hdl);

    ESP_LOGI(TAG, " sampling_count: %d / max: %d", dt_1min_hdl->sampling_count, dt_1min_hdl->samples_maximum_size);

    /* serialize data-table and output in json format every 5-minutes (i.e. 12:00:00, 12:05:00, 12:10:00, etc.) */
    if (time_into_interval(dt_1min_tii_5min_hdl)) {
      // create root object for data-table
      cJSON* dt_1min_json = cJSON_CreateObject();

      // convert the data-table to json object
      datatable_to_json(dt_1min_hdl, &dt_1min_json);

      // render json data-table object to text and print
      char* dt_1min_json_str = cJSON_Print(dt_1min_json);
      ESP_LOGI(TAG, "JSON Data-Table:\n%s", dt_1min_json_str);

      memset(topic, 0x00, sizeof(topic));
      if (BUILD_DEVICE_TOPIC(topic, TOPIC_TYPE_SENSOR) == ESP_OK) {
        ret = publish_sensor_datatable(topic, dt_1min_json_str);
        ESP_LOGI(TAG, "ret : %d", ret);
      }

      if (ret == -1) { /* Qos 0*/  // -1 fail,-2 box full
        char* parsed = parse_rows(dt_1min_json_str);
        if (parsed != NULL) {
          memset(buf, 0x00, sizeof(buf));
          strncpy(buf, parsed, sizeof(buf) - 1);
          ESP_LOGI(TAG, "%s", buf);
          FDATA(BASE_PATH, "%s", buf);  // write to tb data
        } else {
          ESP_LOGW(TAG, "Faild to parse JSON rows");
        }
      }

      // free-up json resources
      cJSON_free(dt_1min_json_str);
      cJSON_Delete(dt_1min_json);
    }
  }

  // free up task resources
  vTaskDelete(NULL);
}
