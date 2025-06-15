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
  uint8_t ec_avg_col;
} teros12_col_t;

typedef struct {
  uint8_t matricPotential_avg_col;
  uint8_t temperature_avg_col;
} teros21_col_t;

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
  teros12_col_t teros12_col;
  teros12_data_t tero12;
  teros21_col_t teros21_col;
  teros21_data_t tero21;
} sensor_datatable_t;

sensor_datatable_t* sensor_dt_instance(void);

#ifdef __cplusplus
}
#endif

#endif
