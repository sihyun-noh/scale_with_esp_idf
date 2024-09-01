#ifndef _MODEL_DEFINE_H_
#define _MODEL_DEFINE_H_

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "scale_read_485.h"
// clang-format off
typedef enum {
    SWII_30KG_200G = 0x01,
    SWII_15KG_100G,
    SWII_6KG_40G,
    SWII_UNKNOWN,
    CB_310G_0_2G,
    CB_3100G_5G,
    EK_4100G_5G,
    CB_12KG_50G,
    CB_UNKNOWN,
    MW2_300G_0_2G,
    MW2_3000G_5G,
    EC_30KG_100G,
    EC_6000G_25G,
    EC_D_30KG_1G,
    EC_D_6KG_0_1G,
    HB_150KG_10G,
    HB_70KG_5G,
    CI_999KG_10G,
    CI_99KG_1G,
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
#define MODEL_SWII_30KG_200G_N "30kg/10g"
#define MODEL_SWII_15KG_100G_N "15kg/5g"
#define MODEL_SWII_6KG_40G_N "6kg/2g"

// AND CB/EK series
#define MODEL_CB_310G_0_2G_N "600g/0.01g"
#define MODEL_CB_3100G_5G_N "3000g/0.1g"
#define MODEL_EK_4100G_5G_N "6000g/0.1g"
#define MODEL_CB_12KG_50G_N "12kg/1g"

// CAS MWII series
#define MODEL_MW2_300G_0_2G_N "300g,0.01g"
#define MODEL_MW2_3000G_5G_N "3000g,0.1g"

// CAS EC series
#define MODEL_EC_30KG_100G_N "30kg,2g"
#define MODEL_EC_6000G_25G_N "6000g,0.5g"

// CAS EC -D series
#define MODEL_EC_D_30KG_100G_N "30kg,1g"
#define MODEL_EC_D_6KG_25G_N "6kg,0.5g"

// CAS HB series
#define MODEL_HB_150KG_500G_N "150kg/10g"
#define MODEL_HB_75KG_250G_N "75kg/5g"

// CAS HB series
#define MODEL_CI_999KG_100G_N "999kg/10g"
#define MODEL_CI_99KG_10G_N "99kg/1g"

model_series_t get_model_series_enum(const char* model_name);

void get_modelSeries_refer(const char* input, char* str, model_series_configured_data* configured);
#ifdef __cplusplus
}
#endif

#endif
