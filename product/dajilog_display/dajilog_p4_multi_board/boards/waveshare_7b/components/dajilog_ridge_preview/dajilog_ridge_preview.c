#include "dajilog_ridge_preview.h"

#include <math.h>
#include <stddef.h>

/** @brief Derive a read-only draft preview from typed product inputs. */
dajilog_ridge_preview_t dajilog_ridge_preview_calculate(
    const dajilog_ridge_settings_t *ridge, bool split_section,
    float north_length_m, float south_length_m)
{
    dajilog_ridge_preview_t result = {0};
    if (ridge == NULL ||
        (ridge->presence_mask & (DAJILOG_RIDGE_PRESENT_WIDTH |
                                 DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE)) !=
            (DAJILOG_RIDGE_PRESENT_WIDTH |
             DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE) ||
        !isfinite(ridge->ridge_width_m) || ridge->ridge_width_m <= 0.0f)
        return result;

    float width = ridge->ridge_width_m;
    if (ridge->work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW) {
        if (!(ridge->presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH) ||
            !isfinite(ridge->included_furrow_width_m) ||
            ridge->included_furrow_width_m < 0.0f)
            return result;
        width += ridge->included_furrow_width_m;
    } else if (ridge->work_width_mode != DAJILOG_WORK_WIDTH_RIDGE_ONLY) {
        return result;
    }
    if (!isfinite(width) || width <= 0.0f) return result;
    result.width_valid = true;
    result.effective_width_m = width;

    float length;
    if (split_section) {
        if (!isfinite(north_length_m) || !isfinite(south_length_m) ||
            north_length_m <= 0.0f || south_length_m <= 0.0f)
            return result;
        length = north_length_m + south_length_m;
    } else {
        if (!(ridge->presence_mask & DAJILOG_RIDGE_PRESENT_SECTION_LENGTH) ||
            !isfinite(ridge->actual_length_m) || ridge->actual_length_m <= 0.0f)
            return result;
        length = ridge->actual_length_m;
    }
    /* Each registered ridge has independent dimensions. Never multiply one
     * ridge's area by the number of profiles. */
    const float area = width * length;
    if (!isfinite(area) || area <= 0.0f) return result;
    result.area_valid = true;
    result.total_area_m2 = area;
    return result;
}
