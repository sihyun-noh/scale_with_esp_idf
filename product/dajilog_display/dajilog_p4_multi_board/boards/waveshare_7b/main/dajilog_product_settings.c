#include "dajilog_product_settings.h"

#include <stddef.h>
#include <string.h>

#include "dajilog_table.h"
#include "dajilog_setting_storage_nvs.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define RIDGE_TABLE_ID        0x07030101U
#define PRESCRIPTION_TABLE_ID 0x07030102U
#define PRODUCT_PROFILE_LIMIT 8U
#define PRODUCT_STORAGE_SCHEMA_VERSION 1U
#define PRODUCT_STORAGE_NAMESPACE      "dajilog_v02"
#define PRODUCT_STORAGE_KEY            "product_cfg"

typedef struct {
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
} product_settings_record_t;

enum {
    RIDGE_FIELD_COUNT = 1,
    RIDGE_FIELD_WIDTH_M,
    RIDGE_FIELD_ACTUAL_LENGTH_M,
};

enum {
    PRESCRIPTION_FIELD_NITROGEN_RATE = 1,
};

static const dajilog_field_schema_t s_ridge_fields[] = {
    {
        .id = RIDGE_FIELD_COUNT,
        .key = "ridge_count",
        .type = DAJILOG_FIELD_U32,
        .offset = offsetof(dajilog_ridge_settings_t, ridge_count),
        .size = sizeof(uint32_t),
        .required = true,
        .has_min = true,
        .has_max = true,
        .min_value.u32 = 1U,
        .max_value.u32 = 999U,
    },
    {
        .id = RIDGE_FIELD_WIDTH_M,
        .key = "ridge_width_m",
        .type = DAJILOG_FIELD_F32,
        .offset = offsetof(dajilog_ridge_settings_t, ridge_width_m),
        .size = sizeof(float),
        .required = true,
        .has_min = true,
        .has_max = true,
        .min_value.f32 = 0.1f,
        .max_value.f32 = 100.0f,
    },
    {
        .id = RIDGE_FIELD_ACTUAL_LENGTH_M,
        .key = "actual_length_m",
        .type = DAJILOG_FIELD_F32,
        .offset = offsetof(dajilog_ridge_settings_t, actual_length_m),
        .size = sizeof(float),
        .required = true,
        .has_min = true,
        .has_max = true,
        .min_value.f32 = 0.1f,
        .max_value.f32 = 100000.0f,
    },
};

static const dajilog_field_schema_t s_prescription_fields[] = {
    {
        .id = PRESCRIPTION_FIELD_NITROGEN_RATE,
        .key = "nitrogen_rate_kg_n_per_10a",
        .type = DAJILOG_FIELD_F32,
        .offset = offsetof(dajilog_prescription_settings_t,
                           nitrogen_rate_kg_n_per_10a),
        .size = sizeof(float),
        .required = true,
        .has_min = true,
        .has_max = true,
        .min_value.f32 = 0.0f,
        .max_value.f32 = 1000.0f,
    },
};

static const dajilog_table_schema_t s_ridge_schema = {
    .id = RIDGE_TABLE_ID,
    .key = "ridge_profiles",
    .schema_version = 1U,
    .row_size = sizeof(dajilog_ridge_settings_t),
    .max_rows = PRODUCT_PROFILE_LIMIT,
    .fields = s_ridge_fields,
    .field_count = sizeof(s_ridge_fields) / sizeof(s_ridge_fields[0]),
};

static const dajilog_table_schema_t s_prescription_schema = {
    .id = PRESCRIPTION_TABLE_ID,
    .key = "prescription_profiles",
    .schema_version = 1U,
    .row_size = sizeof(dajilog_prescription_settings_t),
    .max_rows = PRODUCT_PROFILE_LIMIT,
    .fields = s_prescription_fields,
    .field_count = sizeof(s_prescription_fields) /
                   sizeof(s_prescription_fields[0]),
};

static SemaphoreHandle_t s_lock;
static dajilog_row_id_t s_ridge_row_id;
static dajilog_row_id_t s_prescription_row_id;
static dajilog_ridge_settings_t s_ridge_draft;
static dajilog_prescription_settings_t s_prescription_draft;
static bool s_dirty;
static dajilog_setting_storage_nvs_t s_storage;
static const char *TAG = "DajilogSettings";

/** @brief Register product schemas and build current rows from NVS or defaults. */
esp_err_t dajilog_product_settings_init(void)
{
    if (s_lock != NULL) {
        return ESP_OK;
    }

    esp_err_t error = dajilog_table_engine_init();
    if (error != ESP_OK) {
        return error;
    }
    error = dajilog_table_register(&s_ridge_schema);
    if (error != ESP_OK) {
        return error;
    }
    error = dajilog_table_register(&s_prescription_schema);
    if (error != ESP_OK) {
        return error;
    }

    dajilog_ridge_settings_t initial_ridge = {
        .ridge_count = 6U,
        .ridge_width_m = 1.20f,
        .actual_length_m = 40.0f,
    };
    dajilog_prescription_settings_t initial_prescription = {
        .nitrogen_rate_kg_n_per_10a = 0.0f,
    };
    error = dajilog_setting_storage_nvs_init(
        &s_storage, PRODUCT_STORAGE_NAMESPACE);
    if (error != ESP_OK) {
        return error;
    }
    product_settings_record_t stored = {0};
    error = dajilog_setting_storage_nvs_load(
        &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_SCHEMA_VERSION,
        &stored, sizeof(stored));
    if (error == ESP_OK) {
        error = dajilog_table_validate(
            RIDGE_TABLE_ID, &stored.ridge, sizeof(stored.ridge));
        if (error == ESP_OK) {
            error = dajilog_table_validate(PRESCRIPTION_TABLE_ID,
                                           &stored.prescription,
                                           sizeof(stored.prescription));
        }
        if (error == ESP_OK) {
            initial_ridge = stored.ridge;
            initial_prescription = stored.prescription;
            ESP_LOGI(TAG, "restored product settings from NVS");
        } else {
            ESP_LOGW(TAG,
                     "stored settings failed product validation; defaults used: %s",
                     esp_err_to_name(error));
            error = ESP_OK;
        }
    } else if (error == ESP_ERR_NVS_NOT_FOUND ||
               error == ESP_ERR_NVS_NOT_INITIALIZED) {
        error = ESP_OK;
    } else {
        ESP_LOGW(TAG, "stored settings rejected; defaults used: %s",
                 esp_err_to_name(error));
        error = ESP_OK;
    }

    error = dajilog_table_insert(RIDGE_TABLE_ID, &initial_ridge,
                                 sizeof(initial_ridge), &s_ridge_row_id);
    if (error == ESP_OK) {
        error = dajilog_table_insert(PRESCRIPTION_TABLE_ID,
                                     &initial_prescription,
                                     sizeof(initial_prescription),
                                     &s_prescription_row_id);
    }
    if (error != ESP_OK) {
        return error;
    }

    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
    if (lock == NULL) {
        return ESP_ERR_NO_MEM;
    }
    s_ridge_draft = initial_ridge;
    s_prescription_draft = initial_prescription;
    s_lock = lock;
    return ESP_OK;
}

/** @brief Copy editable drafts and their current table revisions. */
esp_err_t dajilog_product_settings_get_draft(
    dajilog_product_settings_snapshot_t *settings)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (settings == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    settings->ridge = s_ridge_draft;
    settings->prescription = s_prescription_draft;
    settings->dirty = s_dirty;
    xSemaphoreGive(s_lock);

    esp_err_t error = dajilog_table_get_revision(
        RIDGE_TABLE_ID, &settings->ridge_revision);
    if (error == ESP_OK) {
        error = dajilog_table_get_revision(
            PRESCRIPTION_TABLE_ID, &settings->prescription_revision);
    }
    return error;
}

/** @brief Validate and replace only the editable ridge draft. */
esp_err_t dajilog_product_settings_stage_ridge(
    const dajilog_ridge_settings_t *settings)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t error = dajilog_table_validate(
        RIDGE_TABLE_ID, settings, sizeof(*settings));
    if (error != ESP_OK) {
        return error;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_ridge_draft = *settings;
    s_dirty = true;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief Validate and replace only the editable prescription draft. */
esp_err_t dajilog_product_settings_stage_prescription(
    const dajilog_prescription_settings_t *settings)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    esp_err_t error = dajilog_table_validate(
        PRESCRIPTION_TABLE_ID, settings, sizeof(*settings));
    if (error != ESP_OK) {
        return error;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_prescription_draft = *settings;
    s_dirty = true;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief Commit one complete product snapshot, then update both table rows. */
esp_err_t dajilog_product_settings_apply(void)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const dajilog_ridge_settings_t ridge = s_ridge_draft;
    const dajilog_prescription_settings_t prescription =
        s_prescription_draft;
    xSemaphoreGive(s_lock);

    esp_err_t error = dajilog_table_validate(
        RIDGE_TABLE_ID, &ridge, sizeof(ridge));
    if (error == ESP_OK) {
        error = dajilog_table_validate(
            PRESCRIPTION_TABLE_ID, &prescription, sizeof(prescription));
    }
    if (error == ESP_OK) {
        const product_settings_record_t record = {
            .ridge = ridge,
            .prescription = prescription,
        };
        error = dajilog_setting_storage_nvs_save(
            &s_storage, PRODUCT_STORAGE_KEY,
            PRODUCT_STORAGE_SCHEMA_VERSION, &record, sizeof(record));
    }
    if (error == ESP_OK) {
        error = dajilog_table_update(
            RIDGE_TABLE_ID, s_ridge_row_id, &ridge, sizeof(ridge));
    }
    if (error == ESP_OK) {
        error = dajilog_table_update(PRESCRIPTION_TABLE_ID,
                                     s_prescription_row_id,
                                     &prescription,
                                     sizeof(prescription));
    }
    if (error == ESP_OK) {
        xSemaphoreTake(s_lock, portMAX_DELAY);
        s_dirty = false;
        xSemaphoreGive(s_lock);
    }
    return error;
}

/** @brief Restore editable drafts from the current saved RAM table rows. */
esp_err_t dajilog_product_settings_discard(void)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
    esp_err_t error = dajilog_table_get(
        RIDGE_TABLE_ID, s_ridge_row_id, &ridge, sizeof(ridge));
    if (error == ESP_OK) {
        error = dajilog_table_get(PRESCRIPTION_TABLE_ID,
                                  s_prescription_row_id,
                                  &prescription,
                                  sizeof(prescription));
    }
    if (error == ESP_OK) {
        xSemaphoreTake(s_lock, portMAX_DELAY);
        s_ridge_draft = ridge;
        s_prescription_draft = prescription;
        s_dirty = false;
        xSemaphoreGive(s_lock);
    }
    return error;
}
