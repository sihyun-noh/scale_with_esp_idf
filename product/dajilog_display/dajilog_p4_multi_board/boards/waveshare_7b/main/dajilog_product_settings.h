#pragma once

#include "dajilog_product_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the 7B product schemas and restore saved settings from NVS.
 *
 * Valid restored values become both the current table rows and editable drafts.
 * Missing or rejected data falls back to firmware defaults.
 */
esp_err_t dajilog_product_settings_init(void);

/**
 * @brief Copy the current editable settings draft and table revisions.
 * @param[out] settings Destination snapshot owned by the caller.
 */
esp_err_t dajilog_product_settings_get_draft(
    dajilog_product_settings_snapshot_t *settings);

/**
 * @brief Validate and stage ridge settings without writing NVS.
 * @param[in] settings Ridge values entered by the UI.
 */
esp_err_t dajilog_product_settings_stage_ridge(
    const dajilog_ridge_settings_t *settings);

/**
 * @brief Validate and stage fertilizer prescription settings without writing NVS.
 * @param[in] settings Prescription values entered by the UI.
 */
esp_err_t dajilog_product_settings_stage_prescription(
    const dajilog_prescription_settings_t *settings);

/**
 * @brief Persist the complete product draft and update the current table rows.
 *
 * NVS is committed first. RAM rows and the dirty flag change only after the
 * persistent snapshot has been written successfully.
 */
esp_err_t dajilog_product_settings_apply(void);

/**
 * @brief Discard unsaved edits and restore drafts from current table rows.
 */
esp_err_t dajilog_product_settings_discard(void);

#ifdef __cplusplus
}
#endif
