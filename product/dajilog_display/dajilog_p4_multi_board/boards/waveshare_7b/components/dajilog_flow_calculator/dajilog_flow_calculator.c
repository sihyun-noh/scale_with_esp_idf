/** @file 선택 Bxx·처방 제품량·기준속도·채널 배분의 미리보기 계산. */
#include "dajilog_flow_calculator.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

/** @brief 선택 ID에 해당하는 Bxx의 유효 작업폭만 반환한다. */
static dajilog_flow_calc_status_t selected_work_width(
    const dajilog_ridge_profiles_snapshot_t *ridges, double *width)
{
    if (ridges->selected_id == 0U || ridges->count > DAJILOG_RIDGE_PROFILE_LIMIT)
        return DAJILOG_FLOW_CALC_NO_SELECTED_RIDGE;
    for (uint32_t i = 0; i < ridges->count; ++i) {
        if (ridges->profiles[i].id != ridges->selected_id) continue;
        const dajilog_ridge_profile_input_t *ridge = &ridges->profiles[i].input;
        if (!isfinite(ridge->width_m) || ridge->width_m <= 0.0f ||
            ridge->work_width_mode > DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW)
            return DAJILOG_FLOW_CALC_OUT_OF_RANGE;
        double result = ridge->width_m;
        if (ridge->work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW) {
            if (!isfinite(ridge->furrow_width_m) || ridge->furrow_width_m < 0.0f)
                return DAJILOG_FLOW_CALC_OUT_OF_RANGE;
            result += ridge->furrow_width_m;
        }
        if (!isfinite(result) || result <= 0.0 || result > 200.0)
            return DAJILOG_FLOW_CALC_OUT_OF_RANGE;
        *width = result;
        return DAJILOG_FLOW_CALC_OK;
    }
    return DAJILOG_FLOW_CALC_NO_SELECTED_RIDGE;
}

/** @brief 제품량 kg/10a × 작업폭 m × 속도 km/h ÷ 60으로 kg/min을 구한다. */
dajilog_flow_calc_status_t dajilog_flow_calculate_preview(
    const dajilog_ridge_profiles_snapshot_t *ridges,
    const dajilog_zone_settings_t *zone,
    dajilog_flow_target_t *output)
{
    if (output == NULL) return DAJILOG_FLOW_CALC_INCOMPLETE_INPUT;
    memset(output, 0, sizeof(*output));
    if (ridges == NULL || zone == NULL || !zone->configured)
        return DAJILOG_FLOW_CALC_INCOMPLETE_INPUT;
    if (!isfinite(zone->rate_kg_per_10a) || !isfinite(zone->reference_speed_kmh) ||
        zone->rate_kg_per_10a < 0.0f || zone->rate_kg_per_10a > 10000.0f ||
        zone->reference_speed_kmh < 0.0f || zone->reference_speed_kmh > 50.0f ||
        zone->first_channel_permille > 1000U)
        return DAJILOG_FLOW_CALC_OUT_OF_RANGE;
    if (zone->rate_kg_per_10a == 0.0f || zone->reference_speed_kmh == 0.0f)
        return DAJILOG_FLOW_CALC_INCOMPLETE_INPUT;

    double width = 0.0;
    const dajilog_flow_calc_status_t width_status =
        selected_work_width(ridges, &width);
    if (width_status != DAJILOG_FLOW_CALC_OK) return width_status;
    const double total = (double)zone->rate_kg_per_10a * width *
                         zone->reference_speed_kmh / 60.0;
    if (!isfinite(total) || total <= 0.0 || total > 1000.0)
        return DAJILOG_FLOW_CALC_OUT_OF_RANGE;
    const uint32_t total_g_min = (uint32_t)llround(total * 1000.0);
    const uint32_t first = (uint32_t)(
        ((uint64_t)total_g_min * zone->first_channel_permille + 500U) / 1000U);
    output->work_width_m = width;
    output->total_kg_min = total;
    output->channel_target_g_min[0] = first;
    output->channel_target_g_min[1] = total_g_min - first;
    return DAJILOG_FLOW_CALC_OK;
}
