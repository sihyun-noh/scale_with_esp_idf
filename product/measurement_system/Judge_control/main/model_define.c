
#include <stdio.h>
#include <string.h>
#include "model_define.h"
#include "scale_read_485.h"

// clang-format off
const model_group_info_t model_config[] = {
    { .model_id = MODEL_CAS_SW_11, 
      .series_info = {
          { SWII_30KG_200G, MODEL_SWII_30KG_200G_N, 200, 30000, DP_KG_0_001}, 
          { SWII_15KG_100G, MODEL_SWII_15KG_100G_N, 100, 15000, DP_G_1}, 
          { SWII_6KG_40G, MODEL_SWII_6KG_40G_N, 40, 6000, DP_G_1},
      }, 
      .series_count = 3 
    },
    { .model_id = MODEL_AND_CB_12K, 
      .series_info = {
          { CB_310G_0_2G, MODEL_CB_310G_0_2G_N, 0.2, 310, DP_G_0_01}, 
          { CB_3100G_5G, MODEL_CB_3100G_5G_N, 5, 3100, DP_G_0_1}, 
          { EK_4100G_5G, MODEL_EK_4100G_5G_N, 5, 4000, DP_G_0_1}, 
          { CB_12KG_50G, MODEL_CB_12KG_50G_N, 50, 12000, DP_G_0_1}, 
      }, 
    .series_count = 4 
    },
};
// clang-format on
#define NUM_MODELS (sizeof(model_config) / sizeof(model_config[0]))

model_series_t get_model_series_enum(const char* model_name) {
  for (int i = 0; i < NUM_MODELS; i++) {
    for (int j = 0; j < model_config[i].series_count; j++) {
      if (strcmp(model_config[i].series_info[j].refer, model_name) == 0) {
        return model_config[i].series_info[j].series;
      }
    }
  }
  return 0;
}

void get_modelSeries_refer(const char* input, char* str, model_series_configured_data* configured) {
  int model_id, series_id;
  // 입력 값 "7_1"을 분리하여 모델과 시리즈 ID로 변환
  if (sscanf(input, "%d_%d", &model_id, &series_id) != 2) {
    return;
  }
  // swii_model_mappings 배열을 순회하며 일치하는 모델 찾기
  for (int i = 0; i < NUM_MODELS; i++) {
    for (int j = 0; j < model_config[i].series_count; j++) {
      if (model_config[i].model_id == model_id && model_config[i].series_info[j].series == series_id) {
        if (model_id == MODEL_CAS_SW_11) {
          sprintf(str, "SWII/%s", model_config[i].series_info[j].refer);
          configured->model_min = model_config[i].series_info[j].min;
          configured->model_max = model_config[i].series_info[j].max;
          configured->DP = model_config[i].series_info[j].DP;
          configured->series = model_config[i].series_info[j].series;
        } else if (model_id == MODEL_AND_CB_12K) {
          sprintf(str, "AND-CB/%s", model_config[i].series_info[j].refer);
          configured->model_min = model_config[i].series_info[j].min;
          configured->model_max = model_config[i].series_info[j].max;
          configured->DP = model_config[i].series_info[j].DP;
          configured->series = model_config[i].series_info[j].series;
        } else {
          sprintf(str, "none");
        }
        return;
      }
    }
  }

  return;
}
//
// int get_test_model_id() {
//   const char* test_models[] = { MODEL_30KG_10G, MODEL_600G_0_1G, MODEL_15KG_20G, MODEL_10KG_20G };
//
//   for (int i = 0; i < sizeof(test_models) / sizeof(test_models[0]); i++) {
//     swii_model_t model_enum = get_swii_model_enum(test_models[i]);
//     printf("Model: SWII_%s, Enum: %d\n", test_models[i], model_enum);
//   }
//
//   return 1;
// }
