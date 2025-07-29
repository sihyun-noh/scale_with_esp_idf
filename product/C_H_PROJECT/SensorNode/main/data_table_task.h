#ifndef DATA_TABLE_H
#define DATA_TABLE_H

#include "data_table.h"
#include "sdi12_task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  time_into_interval_handle_t handle;
  time_into_interval_config_t config;
} publish_config_t;

typedef struct {
  uint8_t vwc_avg_col;
  uint8_t ta_avg_col;
} teros11_col_t;

typedef struct {
  uint8_t vwc_avg_col;
  uint8_t ta_avg_col;
  uint8_t ec_avg_col;
} teros12_col_t;

typedef struct {
  uint8_t matricPotential_avg_col;
  uint8_t temperature_avg_col;
} teros21_col_t;

typedef struct {
  uint8_t vwc1_col;
  uint8_t temp1_col;
  uint8_t vwc2_col;
  uint8_t temp2_col;
  uint8_t vwc3_col;
  uint8_t temp3_col;
  uint8_t vwc4_col;
  uint8_t temp4_col;
} teros54_col_t;

typedef struct {
  uint8_t windSpeed_col;
  uint8_t windDirection_col;
  uint8_t gustWindSpeed_col;
  uint8_t airTemperature_col;
  uint8_t xOrientation_col;
  uint8_t yOrientation_col;
  uint8_t nullValue_col;
  uint8_t northWindSpeed_col;
  uint8_t eastWindSpeed_col;
} weather_atmos22_col_t;

typedef struct {
  uint8_t address_col;
  uint8_t solar_col;
  uint8_t precipitation_col;
  uint8_t strikes_col;
  uint8_t strikeDistance_col;
  uint8_t windSpeed_col;
  uint8_t windDirection_col;
  uint8_t gustWindSpeed_col;
  uint8_t airTemperature_col;
  uint8_t vaporPressure_col;
  uint8_t atmosphericPressure_col;
  uint8_t relativeHumidity_col;
  uint8_t humiditySensorTemperature_col;
  uint8_t xOrientation_col;
  uint8_t yOrientation_col;
  uint8_t nullValue_col;
  uint8_t northWindSpeed_col;
  uint8_t eastWindSpeed_col;
} weather_at41g2_col_t;

typedef struct {
  uint8_t PPFD;
  uint8_t NDVI_UP_WARD;
  uint8_t NDVI_DOWN_WARD;
} apogee_sensor_col_t;

typedef struct {
  int status;
  datatable_handle_t handle;
  datatable_config_t config;
  publish_config_t publish_interval;
  // 포트별 컬럼 인덱스
  uint8_t pa_avg_col;
  uint8_t ta_avg_col;
  uint8_t ta_min_col;
  uint8_t ta_max_col;
  uint8_t td_avg_col;
  uint8_t pressure_col;
  uint8_t radiation_col;
  weather_at41g2_data_t at41g2;
  weather_at41g2_col_t at41g2_col;
  weather_atmos22_data_t atmos22;
  weather_atmos22_col_t atmos22_col;
  teros11_col_t teros11_col;
  teros11_data_t tero11;
  teros12_col_t teros12_col;
  teros12_data_t tero12;
  teros21_col_t teros21_col;
  teros21_data_t tero21;
  teros54_col_t teros54_col;
  teros54_data_t tero54;
  apogee_sensor_t apogee_data;
  apogee_sensor_col_t apogee_data_col;

} sensor_datatable_t;

sensor_datatable_t* sensor_dt_instance(void);

#ifdef __cplusplus
}
#endif

#endif
