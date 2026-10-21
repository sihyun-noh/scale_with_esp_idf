#pragma once

#include "dajilog_product_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t dajilog_product_settings_init(void);
esp_err_t dajilog_product_settings_get_draft(
    dajilog_product_settings_snapshot_t *settings);
esp_err_t dajilog_product_settings_stage_ridge(
    const dajilog_ridge_settings_t *settings);
esp_err_t dajilog_product_settings_stage_prescription(
    const dajilog_prescription_settings_t *settings);
esp_err_t dajilog_product_settings_apply(void);
esp_err_t dajilog_product_settings_discard(void);

#ifdef __cplusplus
}
#endif
