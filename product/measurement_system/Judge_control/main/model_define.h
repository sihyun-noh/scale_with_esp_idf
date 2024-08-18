#ifndef _MODEL_DEFINE_H_
#define _MODEL_DEFINE_H_

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "scale_read_485.h"
// clang-format off
typedef enum {
    SWII_30KG_200G,
    SWII_15KG_100G,
    SWII_6KG_40G,
    SWII_UNKNOWN,
    CB_310G_0_2G,
    CB_3100G_5G,
    EK_4100G_5G,
    CB_12KG_50G,
    CB_UNKNOWN,
} model_series_t;
// clang-format on

typedef struct {
  model_series_t series;
  const char* refer;
  float min;
  int32_t max;
  decimal_point_t DP;
} model_series_info_t;

typedef struct {
  indicator_model_t model_id;
  model_series_info_t series_info[10];
  uint8_t series_count;
} model_group_info_t;

typedef struct {
  float model_min;
  int32_t model_max;
  decimal_point_t DP;
  model_series_t series;
} model_series_configured_data;

// CAS SWII series
#define MODEL_SWII_30KG_200G_N "30kg/200g"
#define MODEL_SWII_15KG_100G_N "15kg/100g"
#define MODEL_SWII_6KG_40G_N "6kg/40g"

// AND SWII series
#define MODEL_CB_310G_0_2G_N "310g/0.2g"
#define MODEL_CB_3100G_5G_N "3100g/5g"
#define MODEL_EK_4100G_5G_N "4000g/5g"
#define MODEL_CB_12KG_50G_N "12kg/50g"

model_series_t get_model_series_enum(const char* model_name);

void get_modelSeries_refer(const char* input, char* str, model_series_configured_data* configured);
#ifdef __cplusplus
}
#endif

#endif
