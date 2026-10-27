#pragma once

#include <stdbool.h>

#include "dajilog_product_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool width_valid;
    bool area_valid;
    float effective_width_m;
    float total_area_m2;
} dajilog_ridge_preview_t;

/** @brief Calculate draft width/area only; never authorize RUN or motor output. */
dajilog_ridge_preview_t dajilog_ridge_preview_calculate(
    const dajilog_ridge_settings_t *ridge, bool split_section,
    float north_length_m, float south_length_m);

#ifdef __cplusplus
}
#endif
