#include <assert.h>
#include <math.h>

#include "dajilog_ridge_preview.h"

int main(void)
{
    dajilog_ridge_settings_t ridge = {
        .presence_mask = DAJILOG_RIDGE_PRESENT_COUNT |
                         DAJILOG_RIDGE_PRESENT_WIDTH |
                         DAJILOG_RIDGE_PRESENT_SECTION_LENGTH |
                         DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE,
        .ridge_count = 2,
        .ridge_width_m = 1.2f,
        .actual_length_m = 100.0f,
        .work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_ONLY,
    };
    dajilog_ridge_preview_t result =
        dajilog_ridge_preview_calculate(&ridge, false, 0.0f, 0.0f);
    assert(result.width_valid && result.area_valid);
    assert(fabsf(result.effective_width_m - 1.2f) < 0.001f);
    assert(fabsf(result.total_area_m2 - 120.0f) < 0.01f);

    ridge.work_width_mode = DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW;
    result = dajilog_ridge_preview_calculate(&ridge, false, 0.0f, 0.0f);
    assert(!result.width_valid && !result.area_valid);
    ridge.presence_mask |= DAJILOG_RIDGE_PRESENT_FURROW_WIDTH;
    ridge.included_furrow_width_m = 0.3f;
    result = dajilog_ridge_preview_calculate(&ridge, false, 0.0f, 0.0f);
    assert(result.width_valid && result.area_valid);
    assert(fabsf(result.total_area_m2 - 150.0f) < 0.01f);

    result = dajilog_ridge_preview_calculate(&ridge, true, 40.0f, 60.0f);
    assert(result.width_valid && result.area_valid);
    assert(fabsf(result.total_area_m2 - 150.0f) < 0.01f);
    result = dajilog_ridge_preview_calculate(&ridge, true, 0.0f, 60.0f);
    assert(result.width_valid && !result.area_valid);
    ridge.ridge_width_m = NAN;
    result = dajilog_ridge_preview_calculate(&ridge, false, 0.0f, 0.0f);
    assert(!result.width_valid && !result.area_valid);
    return 0;
}
