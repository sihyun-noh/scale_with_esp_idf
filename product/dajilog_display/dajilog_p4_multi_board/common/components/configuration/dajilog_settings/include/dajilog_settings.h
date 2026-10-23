#pragma once

#include <stdbool.h>

#include "dajilog_setting_schema.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Initialize legacy settings from NVS or schema defaults. Idempotent. */
esp_err_t dajilog_settings_init(void);

/** @brief Report whether the legacy settings service is ready. */
bool dajilog_settings_is_initialized(void);

/** @brief Copy the currently applied value for one setting. */
esp_err_t dajilog_settings_get(
    dajilog_setting_id_t id,
    dajilog_setting_value_t *value);

/** @brief Copy the editable draft value for one setting. */
esp_err_t dajilog_settings_get_draft(
    dajilog_setting_id_t id,
    dajilog_setting_value_t *value);

/** @brief Validate and update only the editable draft value. */
esp_err_t dajilog_settings_set_draft(
    dajilog_setting_id_t id,
    const dajilog_setting_value_t *value);

/** @brief Replace one current value with its validated draft value. */
esp_err_t dajilog_settings_apply(dajilog_setting_id_t id);

/** @brief Validate every draft first, then apply all drafts in RAM. */
esp_err_t dajilog_settings_apply_all(void);

/**
 * @brief Validate every draft, persist entries to NVS, then apply them.
 *
 * Persistent entries are committed before current RAM values are replaced.
 * replace all current values. Current values stay unchanged if NVS fails.
 */
esp_err_t dajilog_settings_save_all(void);

/** @brief Restore one editable draft from its current value. */
esp_err_t dajilog_settings_discard_draft(dajilog_setting_id_t id);
/** @brief Restore every editable draft from its current value. */
esp_err_t dajilog_settings_discard_all_drafts(void);

/** @brief Report whether one draft differs from its current value. */
bool dajilog_settings_is_dirty(dajilog_setting_id_t id);

/** @brief Read one current setting as a float with type checking. */
esp_err_t dajilog_settings_get_float(
    dajilog_setting_id_t id,
    float *value);

/** @brief Read one current setting as an integer with type checking. */
esp_err_t dajilog_settings_get_int(
    dajilog_setting_id_t id,
    int32_t *value);

/** @brief Stage one float draft after schema validation. */
esp_err_t dajilog_settings_set_draft_float(
    dajilog_setting_id_t id,
    float value);

/** @brief Stage one integer draft after schema validation. */
esp_err_t dajilog_settings_set_draft_int(
    dajilog_setting_id_t id,
    int32_t value);

#ifdef __cplusplus
}
#endif
