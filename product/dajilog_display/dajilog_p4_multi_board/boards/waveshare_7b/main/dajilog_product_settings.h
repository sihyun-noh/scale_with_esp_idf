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
 * Missing or rejected data remains UNSET; example values are never installed
 * as firmware defaults.
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
/** @brief Stage one validated 7B setting zone in RAM. */
esp_err_t dajilog_product_settings_stage_zone(
    dajilog_setting_zone_id_t zone, const dajilog_zone_settings_t *settings);

/**
 * @brief Persist the complete product draft and update the current table rows.
 *
 * NVS is committed first. RAM rows and the dirty flag change only after the
 * persistent snapshot has been written successfully.
 */
esp_err_t dajilog_product_settings_save(void);

/**
 * @brief Discard unsaved edits and restore drafts from current table rows.
 */
esp_err_t dajilog_product_settings_discard(void);

/** @brief Confirm the saved configuration for use by the next RUN. */
esp_err_t dajilog_product_settings_confirm(void);

/** @brief Copy the confirmed applied configuration for a RUN context. */
esp_err_t dajilog_product_settings_get_applied(
    dajilog_product_settings_snapshot_t *settings);

/** @brief Copy independently registered ridge profiles and selection. */
esp_err_t dajilog_product_settings_get_ridge_profiles(
    dajilog_ridge_profiles_snapshot_t *profiles);
/** @brief Validate and persist one new profile; called by Storage Task only. */
esp_err_t dajilog_product_settings_add_ridge_profile(
    const dajilog_ridge_profile_input_t *input);
/** @brief Persist a selected registered profile; called by Storage Task only. */
esp_err_t dajilog_product_settings_select_ridge_profile(uint32_t id);
/** @brief Persist deletion of one Bxx ID without renumbering survivors. */
esp_err_t dajilog_product_settings_delete_ridge_profile(uint32_t id);

#ifdef __cplusplus
}
#endif
