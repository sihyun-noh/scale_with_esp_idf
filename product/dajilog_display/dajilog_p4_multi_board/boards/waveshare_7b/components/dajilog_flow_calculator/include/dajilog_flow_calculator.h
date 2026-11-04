#pragma once

#include "dajilog_product_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DAJILOG_FLOW_CALC_OK = 0,
    DAJILOG_FLOW_CALC_NO_SELECTED_RIDGE,
    DAJILOG_FLOW_CALC_INCOMPLETE_INPUT,
    DAJILOG_FLOW_CALC_OUT_OF_RANGE,
} dajilog_flow_calc_status_t;

typedef struct {
    double work_width_m;
    double total_kg_min;
    uint32_t channel_target_g_min[2];
} dajilog_flow_target_t;

/**
 * @brief 선택한 Bxx 한 개와 S1/S2의 처방 제품량·기준속도로 채널별 목표를 계산한다.
 *
 * 결과는 미리보기용이며 NVS·UI·CAN을 변경하지 않는다. 두둑 길이나 등록 개수는
 * 순간 유량에 곱하지 않는다. 실패하면 output은 0으로 유지된다.
 */
dajilog_flow_calc_status_t dajilog_flow_calculate_preview(
    const dajilog_ridge_profiles_snapshot_t *ridges,
    const dajilog_zone_settings_t *zone,
    dajilog_flow_target_t *output);

#ifdef __cplusplus
}
#endif
