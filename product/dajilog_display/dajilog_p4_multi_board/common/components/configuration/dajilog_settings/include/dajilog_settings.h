#pragma once

#include <stdbool.h>

#include "dajilog_setting_schema.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the in-memory setting table with schema defaults. Idempotent. */
esp_err_t dajilog_settings_init(void);

bool dajilog_settings_is_initialized(void);

esp_err_t dajilog_settings_get(
    dajilog_setting_id_t id,
    dajilog_setting_value_t *value);

esp_err_t dajilog_settings_get_draft(
    dajilog_setting_id_t id,
    dajilog_setting_value_t *value);

/** Validate and update only the editable draft value. */
esp_err_t dajilog_settings_set_draft(
    dajilog_setting_id_t id,
    const dajilog_setting_value_t *value);

/** Atomically replace the current value with one validated draft value. */
esp_err_t dajilog_settings_apply(dajilog_setting_id_t id);

/** Validate every draft first, then apply all drafts as one transaction. */
esp_err_t dajilog_settings_apply_all(void);

/**
 * Validate every draft, persist persistent entries to NVS, then atomically
 * replace all current values. Current values stay unchanged if NVS fails.
 */
esp_err_t dajilog_settings_save_all(void);

esp_err_t dajilog_settings_discard_draft(dajilog_setting_id_t id);
esp_err_t dajilog_settings_discard_all_drafts(void);

bool dajilog_settings_is_dirty(dajilog_setting_id_t id);

esp_err_t dajilog_settings_get_float(
    dajilog_setting_id_t id,
    float *value);

esp_err_t dajilog_settings_get_int(
    dajilog_setting_id_t id,
    int32_t *value);

esp_err_t dajilog_settings_set_draft_float(
    dajilog_setting_id_t id,
    float value);

esp_err_t dajilog_settings_set_draft_int(
    dajilog_setting_id_t id,
    int32_t value);

#ifdef __cplusplus
}
#endif
