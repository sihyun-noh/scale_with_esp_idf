/** @file 처방 카드의 질소 기준·직접 제품량을 전층/지중 kg/10a로 변환한다. */
#include "dajilog_prescription_calculator.h"

#include <math.h>
#include <string.h>

#include "dajilog_product_catalog.h"

/** @brief 입력 모드별 필수 필드가 있는지 확인한다. */
static bool has_required_fields(const dajilog_prescription_settings_t *value)
{
    uint32_t required = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE;
    if (value->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN) {
        required |= DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET |
                    DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO;
    } else if (value->input_mode == DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT) {
        required |= DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT |
                    DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT;
    } else {
        return false;
    }
    return (value->presence_mask & required) == required;
}

/** @brief 처방 Draft의 두 구역 제품량을 부작용 없이 계산한다. */
dajilog_prescription_calc_status_t dajilog_prescription_calculate_product_rates(
    const dajilog_prescription_settings_t *prescription,
    const char product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY],
    dajilog_prescription_product_rates_t *output)
{
    if (output == NULL) return DAJILOG_PRESCRIPTION_CALC_INVALID_VALUE;
    memset(output, 0, sizeof(*output));
    if (prescription == NULL || product_codes == NULL)
        return DAJILOG_PRESCRIPTION_CALC_INVALID_VALUE;
    if (!has_required_fields(prescription))
        return DAJILOG_PRESCRIPTION_CALC_MISSING_INPUT;

    const dajilog_product_entry_t *products[DAJILOG_SETTING_ZONE_COUNT];
    for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
        if (product_codes[i][0] == '\0')
            return DAJILOG_PRESCRIPTION_CALC_MISSING_INPUT;
        products[i] = dajilog_product_catalog_find(product_codes[i]);
        if (products[i] == NULL)
            return DAJILOG_PRESCRIPTION_CALC_PRODUCT_NOT_FOUND;
    }

    float surface;
    float subsurface;
    if (prescription->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN) {
        const float nitrogen = prescription->nitrogen_rate_kg_n_per_10a;
        const float ratio = prescription->surface_nitrogen_ratio_pct;
        const float surface_pct = products[DAJILOG_SETTING_ZONE_SURFACE]->nitrogen_percent;
        const float subsurface_pct = products[DAJILOG_SETTING_ZONE_SUBSURFACE]->nitrogen_percent;
        if (!isfinite(nitrogen) || nitrogen < 0.0f || nitrogen > 1000.0f ||
            !isfinite(ratio) || ratio < 0.0f || ratio > 100.0f ||
            !isfinite(surface_pct) || surface_pct <= 0.0f || surface_pct > 100.0f ||
            !isfinite(subsurface_pct) || subsurface_pct <= 0.0f || subsurface_pct > 100.0f)
            return DAJILOG_PRESCRIPTION_CALC_INVALID_VALUE;

        surface = nitrogen * (ratio / 100.0f) / (surface_pct / 100.0f);
        subsurface = nitrogen * ((100.0f - ratio) / 100.0f) /
                     (subsurface_pct / 100.0f);
    } else {
        surface = prescription->surface_product_rate_kg_per_10a;
        subsurface = prescription->subsurface_product_rate_kg_per_10a;
    }
    if (!isfinite(surface) || !isfinite(subsurface) || surface < 0.0f ||
        subsurface < 0.0f || surface > 10000.0f || subsurface > 10000.0f)
        return DAJILOG_PRESCRIPTION_CALC_INVALID_VALUE;

    output->surface_product_kg_per_10a = surface;
    output->subsurface_product_kg_per_10a = subsurface;
    return DAJILOG_PRESCRIPTION_CALC_OK;
}
