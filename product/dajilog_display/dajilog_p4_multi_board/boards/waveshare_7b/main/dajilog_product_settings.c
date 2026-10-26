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
#define PRODUCT_STORAGE_SCHEMA_VERSION 3U
#define PRODUCT_STORAGE_LEGACY_VERSION 1U
#define PRODUCT_STORAGE_NAMESPACE      "dajilog_v02"
#define PRODUCT_STORAGE_KEY            "product_cfg"

typedef struct {
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
    dajilog_setting_readiness_t saved_readiness;
    bool confirmed;
    uint32_t saved_revision;
} product_settings_record_t;

typedef struct {
    struct {
        uint32_t ridge_count;
        float ridge_width_m;
        float actual_length_m;
    } ridge;
    struct {
        float nitrogen_rate_kg_n_per_10a;
    } prescription;
} product_settings_legacy_record_t;

enum {
    RIDGE_FIELD_PRESENCE = 1,
    RIDGE_FIELD_COUNT,
    RIDGE_FIELD_WIDTH_M,
    RIDGE_FIELD_ACTUAL_LENGTH_M,
    RIDGE_FIELD_FURROW_WIDTH_M,
    RIDGE_FIELD_WORK_WIDTH_MODE,
};

enum {
    PRESCRIPTION_FIELD_PRESENCE = 1,
    PRESCRIPTION_FIELD_INPUT_MODE,
    PRESCRIPTION_FIELD_NITROGEN_RATE,
    PRESCRIPTION_FIELD_SURFACE_RATIO,
    PRESCRIPTION_FIELD_SURFACE_N_PCT,
    PRESCRIPTION_FIELD_SUBSURFACE_N_PCT,
    PRESCRIPTION_FIELD_SURFACE_PRODUCT,
    PRESCRIPTION_FIELD_SUBSURFACE_PRODUCT,
    PRESCRIPTION_FIELD_MANUAL_SURFACE,
    PRESCRIPTION_FIELD_MANUAL_SUBSURFACE,
};

static const dajilog_field_schema_t s_ridge_fields[] = {
    {
        .id = RIDGE_FIELD_PRESENCE,
        .key = "presence_mask",
        .type = DAJILOG_FIELD_U32,
        .offset = offsetof(dajilog_ridge_settings_t, presence_mask),
        .size = sizeof(uint32_t),
    },
    {
        .id = RIDGE_FIELD_COUNT,
        .key = "ridge_count",
        .type = DAJILOG_FIELD_U32,
        .offset = offsetof(dajilog_ridge_settings_t, ridge_count),
        .size = sizeof(uint32_t),
        .required = true,
        .has_min = true,
        .has_max = true,
        .min_value.u32 = 0U,
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
        .min_value.f32 = 0.0f,
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
        .min_value.f32 = 0.0f,
        .max_value.f32 = 100000.0f,
    },
    {
        .id = RIDGE_FIELD_FURROW_WIDTH_M,
        .key = "included_furrow_width_m",
        .type = DAJILOG_FIELD_F32,
        .offset = offsetof(dajilog_ridge_settings_t, included_furrow_width_m),
        .size = sizeof(float),
        .has_min = true,
        .has_max = true,
        .min_value.f32 = 0.0f,
        .max_value.f32 = 100.0f,
    },
    {
        .id = RIDGE_FIELD_WORK_WIDTH_MODE,
        .key = "work_width_mode",
        .type = DAJILOG_FIELD_ENUM,
        .offset = offsetof(dajilog_ridge_settings_t, work_width_mode),
        .size = sizeof(int32_t),
        .has_min = true,
        .has_max = true,
        .min_value.i32 = DAJILOG_WORK_WIDTH_RIDGE_ONLY,
        .max_value.i32 = DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW,
    },
};

static const dajilog_field_schema_t s_prescription_fields[] = {
    {
        .id = PRESCRIPTION_FIELD_PRESENCE,
        .key = "presence_mask",
        .type = DAJILOG_FIELD_U32,
        .offset = offsetof(dajilog_prescription_settings_t, presence_mask),
        .size = sizeof(uint32_t),
    },
    {
        .id = PRESCRIPTION_FIELD_INPUT_MODE,
        .key = "input_mode",
        .type = DAJILOG_FIELD_ENUM,
        .offset = offsetof(dajilog_prescription_settings_t, input_mode),
        .size = sizeof(int32_t),
        .has_min = true,
        .has_max = true,
        .min_value.i32 = DAJILOG_PRESCRIPTION_BY_NITROGEN,
        .max_value.i32 = DAJILOG_PRESCRIPTION_BY_PRODUCT_AMOUNT,
    },
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
#define PRESCRIPTION_F32_FIELD(field_id, field_key, member, maximum)         \
    {                                                                        \
        .id = field_id, .key = field_key, .type = DAJILOG_FIELD_F32,        \
        .offset = offsetof(dajilog_prescription_settings_t, member),         \
        .size = sizeof(float), .has_min = true, .has_max = true,             \
        .min_value.f32 = 0.0f, .max_value.f32 = maximum,                     \
    }
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_SURFACE_RATIO,
                           "surface_nitrogen_ratio_pct",
                           surface_nitrogen_ratio_pct, 100.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_SURFACE_N_PCT,
                           "surface_fertilizer_nitrogen_pct",
                           surface_fertilizer_nitrogen_pct, 100.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_SUBSURFACE_N_PCT,
                           "subsurface_fertilizer_nitrogen_pct",
                           subsurface_fertilizer_nitrogen_pct, 100.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_SURFACE_PRODUCT,
                           "surface_product_rate_kg_per_10a",
                           surface_product_rate_kg_per_10a, 10000.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_SUBSURFACE_PRODUCT,
                           "subsurface_product_rate_kg_per_10a",
                           subsurface_product_rate_kg_per_10a, 10000.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_MANUAL_SURFACE,
                           "manual_surface_rate_kg_min",
                           manual_surface_rate_kg_min, 10000.0f),
    PRESCRIPTION_F32_FIELD(PRESCRIPTION_FIELD_MANUAL_SUBSURFACE,
                           "manual_subsurface_rate_kg_min",
                           manual_subsurface_rate_kg_min, 10000.0f),
#undef PRESCRIPTION_F32_FIELD
};

static esp_err_t validate_ridge_row(const void *row, size_t row_size,
                                    void *context)
{
    (void)context;
    if (row == NULL || row_size != sizeof(dajilog_ridge_settings_t)) {
        return ESP_ERR_INVALID_ARG;
    }
    const dajilog_ridge_settings_t *value = row;
    const uint32_t known_mask = DAJILOG_RIDGE_PRESENT_COUNT |
                                DAJILOG_RIDGE_PRESENT_WIDTH |
                                DAJILOG_RIDGE_PRESENT_FURROW_WIDTH |
                                DAJILOG_RIDGE_PRESENT_SECTION_LENGTH |
                                DAJILOG_RIDGE_PRESENT_WORK_WIDTH_MODE;
    if ((value->presence_mask & ~known_mask) != 0U) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((value->presence_mask & DAJILOG_RIDGE_PRESENT_COUNT) != 0U &&
        value->ridge_count == 0U) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((value->presence_mask & DAJILOG_RIDGE_PRESENT_WIDTH) != 0U &&
        value->ridge_width_m <= 0.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((value->presence_mask & DAJILOG_RIDGE_PRESENT_SECTION_LENGTH) != 0U &&
        value->actual_length_m <= 0.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    if ((value->presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH) != 0U &&
        value->included_furrow_width_m < 0.0f) {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

static esp_err_t validate_prescription_row(const void *row, size_t row_size,
                                           void *context)
{
    (void)context;
    if (row == NULL || row_size != sizeof(dajilog_prescription_settings_t)) {
        return ESP_ERR_INVALID_ARG;
    }
    const dajilog_prescription_settings_t *value = row;
    const uint32_t known_mask =
        DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE |
        DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET |
        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO |
        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT |
        DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT |
        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT |
        DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT |
        DAJILOG_PRESCRIPTION_PRESENT_MANUAL_SURFACE |
        DAJILOG_PRESCRIPTION_PRESENT_MANUAL_SUBSURFACE;
    if ((value->presence_mask & ~known_mask) != 0U) {
        return ESP_ERR_INVALID_ARG;
    }
    if (value->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN &&
        (value->presence_mask & DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET) != 0U &&
        value->nitrogen_rate_kg_n_per_10a > 0.0f &&
        (((value->presence_mask & DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT) != 0U &&
          value->surface_fertilizer_nitrogen_pct <= 0.0f) ||
         ((value->presence_mask & DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT) != 0U &&
          value->subsurface_fertilizer_nitrogen_pct <= 0.0f))) {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}

static dajilog_setting_readiness_t calculate_readiness(
    const dajilog_ridge_settings_t *ridge,
    const dajilog_prescription_settings_t *prescription,
    bool confirmed)
{
    if ((ridge->presence_mask & DAJILOG_RIDGE_REQUIRED_MASK) !=
        DAJILOG_RIDGE_REQUIRED_MASK) {
        return DAJILOG_SETTING_UNSET;
    }
    if (ridge->work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW &&
        (ridge->presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH) == 0U) {
        return DAJILOG_SETTING_UNSET;
    }

    uint32_t required = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE |
                        DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO;
    if (prescription->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN) {
        required |= DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET |
                    DAJILOG_PRESCRIPTION_PRESENT_SURFACE_N_PCT |
                    DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_N_PCT;
    } else {
        required |= DAJILOG_PRESCRIPTION_PRESENT_SURFACE_PRODUCT |
                    DAJILOG_PRESCRIPTION_PRESENT_SUBSURFACE_PRODUCT;
    }
    if ((prescription->presence_mask & required) != required) {
        return DAJILOG_SETTING_UNSET;
    }
    return confirmed ? DAJILOG_SETTING_CONFIRMED
                     : DAJILOG_SETTING_CONFIGURED;
}

static const dajilog_table_schema_t s_ridge_schema = {
    .id = RIDGE_TABLE_ID,
    .key = "ridge_profiles",
    .schema_version = 3U,
    .row_size = sizeof(dajilog_ridge_settings_t),
    .max_rows = PRODUCT_PROFILE_LIMIT,
    .fields = s_ridge_fields,
    .field_count = sizeof(s_ridge_fields) / sizeof(s_ridge_fields[0]),
    .validator = validate_ridge_row,
};

static const dajilog_table_schema_t s_prescription_schema = {
    .id = PRESCRIPTION_TABLE_ID,
    .key = "prescription_profiles",
    .schema_version = 3U,
    .row_size = sizeof(dajilog_prescription_settings_t),
    .max_rows = PRODUCT_PROFILE_LIMIT,
    .fields = s_prescription_fields,
    .field_count = sizeof(s_prescription_fields) /
                   sizeof(s_prescription_fields[0]),
    .validator = validate_prescription_row,
};

static SemaphoreHandle_t s_lock;
static dajilog_row_id_t s_ridge_row_id;
static dajilog_row_id_t s_prescription_row_id;
static dajilog_ridge_settings_t s_ridge_draft;
static dajilog_prescription_settings_t s_prescription_draft;
static bool s_dirty;
static dajilog_setting_readiness_t s_saved_readiness;
static bool s_confirmed;
static uint32_t s_saved_revision;
static uint32_t s_applied_revision;
static dajilog_setting_storage_nvs_t s_storage;
static const char *TAG = "DajilogSettings";

/** @brief Register v0.3 schemas and restore saved or migrated settings. */
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

    dajilog_ridge_settings_t initial_ridge = {0};
    dajilog_prescription_settings_t initial_prescription = {0};
    dajilog_setting_readiness_t initial_readiness = DAJILOG_SETTING_UNSET;
    bool initial_confirmed = false;
    uint32_t initial_saved_revision = 0U;
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
            initial_confirmed = stored.confirmed;
            initial_saved_revision = stored.saved_revision;
            initial_readiness = calculate_readiness(
                &initial_ridge, &initial_prescription, initial_confirmed);
            initial_confirmed =
                initial_readiness == DAJILOG_SETTING_CONFIRMED;
            ESP_LOGI(TAG, "restored v0.3 product settings from NVS");
        } else {
            ESP_LOGW(TAG,
                     "stored v0.3 settings failed validation; UNSET used: %s",
                     esp_err_to_name(error));
            error = ESP_OK;
        }
    } else {
        product_settings_legacy_record_t legacy = {0};
        const esp_err_t legacy_error = dajilog_setting_storage_nvs_load(
            &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_LEGACY_VERSION,
            &legacy, sizeof(legacy));
        if (legacy_error == ESP_OK) {
            initial_ridge.presence_mask = DAJILOG_RIDGE_PRESENT_COUNT |
                                          DAJILOG_RIDGE_PRESENT_WIDTH |
                                          DAJILOG_RIDGE_PRESENT_SECTION_LENGTH;
            initial_ridge.ridge_count = legacy.ridge.ridge_count;
            initial_ridge.ridge_width_m = legacy.ridge.ridge_width_m;
            initial_ridge.actual_length_m = legacy.ridge.actual_length_m;
            initial_prescription.presence_mask =
                DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE |
                DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET;
            initial_prescription.input_mode =
                DAJILOG_PRESCRIPTION_BY_NITROGEN;
            initial_prescription.nitrogen_rate_kg_n_per_10a =
                legacy.prescription.nitrogen_rate_kg_n_per_10a;
            initial_saved_revision = 1U;
            ESP_LOGW(TAG,
                     "migrated v0.2 values as partial, unconfirmed v0.3 settings");
        } else if (error != ESP_ERR_NVS_NOT_FOUND &&
                   error != ESP_ERR_NVS_NOT_INITIALIZED &&
                   error != ESP_ERR_INVALID_SIZE &&
                   error != ESP_ERR_INVALID_VERSION) {
            ESP_LOGW(TAG, "stored settings rejected; UNSET used: %s",
                     esp_err_to_name(error));
        }
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
    s_saved_readiness = initial_readiness;
    s_confirmed = initial_confirmed;
    s_saved_revision = initial_saved_revision;
    s_applied_revision = initial_confirmed ? initial_saved_revision : 0U;
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
    settings->draft_readiness = calculate_readiness(
        &s_ridge_draft, &s_prescription_draft, false);
    settings->saved_readiness = s_saved_readiness;
    settings->saved_revision = s_saved_revision;
    settings->applied_revision = s_applied_revision;
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
    const dajilog_setting_readiness_t readiness =
        calculate_readiness(&ridge, &prescription, false);
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const uint32_t current_revision = s_saved_revision;
    xSemaphoreGive(s_lock);
    const uint32_t incremented_revision = current_revision + 1U;
    const uint32_t next_revision =
        incremented_revision == 0U ? 1U : incremented_revision;
    if (error == ESP_OK) {
        const product_settings_record_t record = {
            .ridge = ridge,
            .prescription = prescription,
            .saved_readiness = readiness,
            .confirmed = false,
            .saved_revision = next_revision,
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
        s_saved_readiness = readiness;
        s_confirmed = false;
        s_saved_revision = next_revision;
        s_applied_revision = 0U;
        xSemaphoreGive(s_lock);
    }
    return error;
}

/** @brief Mark a complete saved configuration as confirmed and applied. */
esp_err_t dajilog_product_settings_confirm(void)
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
                                  &prescription, sizeof(prescription));
    }
    if (error != ESP_OK ||
        calculate_readiness(&ridge, &prescription, false) !=
            DAJILOG_SETTING_CONFIGURED) {
        return error != ESP_OK ? error : ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    const uint32_t saved_revision = s_saved_revision;
    xSemaphoreGive(s_lock);
    const product_settings_record_t record = {
        .ridge = ridge,
        .prescription = prescription,
        .saved_readiness = DAJILOG_SETTING_CONFIRMED,
        .confirmed = true,
        .saved_revision = saved_revision,
    };
    error = dajilog_setting_storage_nvs_save(
        &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_SCHEMA_VERSION,
        &record, sizeof(record));
    if (error == ESP_OK) {
        xSemaphoreTake(s_lock, portMAX_DELAY);
        s_confirmed = true;
        s_saved_readiness = DAJILOG_SETTING_CONFIRMED;
        s_applied_revision = saved_revision;
        xSemaphoreGive(s_lock);
    }
    return error;
}

/** @brief Return only a confirmed configuration for RUN context latching. */
esp_err_t dajilog_product_settings_get_applied(
    dajilog_product_settings_snapshot_t *settings)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    if (settings == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    const bool confirmed = s_confirmed;
    const uint32_t saved_revision = s_saved_revision;
    const uint32_t applied_revision = s_applied_revision;
    xSemaphoreGive(s_lock);
    if (!confirmed || applied_revision == 0U) {
        return ESP_ERR_INVALID_STATE;
    }

    memset(settings, 0, sizeof(*settings));
    esp_err_t error = dajilog_table_get(
        RIDGE_TABLE_ID, s_ridge_row_id, &settings->ridge,
        sizeof(settings->ridge));
    if (error == ESP_OK) {
        error = dajilog_table_get(PRESCRIPTION_TABLE_ID,
                                  s_prescription_row_id,
                                  &settings->prescription,
                                  sizeof(settings->prescription));
    }
    if (error == ESP_OK) {
        settings->draft_readiness = DAJILOG_SETTING_CONFIRMED;
        settings->saved_readiness = DAJILOG_SETTING_CONFIRMED;
        settings->saved_revision = saved_revision;
        settings->applied_revision = applied_revision;
        (void)dajilog_table_get_revision(RIDGE_TABLE_ID,
                                         &settings->ridge_revision);
        (void)dajilog_table_get_revision(PRESCRIPTION_TABLE_ID,
                                         &settings->prescription_revision);
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
