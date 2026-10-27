#include "dajilog_product_settings.h"

#include <stddef.h>
#include <string.h>
#include <math.h>

#include "dajilog_table.h"
#include "dajilog_setting_storage_nvs.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define RIDGE_TABLE_ID        0x07030101U
#define PRESCRIPTION_TABLE_ID 0x07030102U
#define PRODUCT_PROFILE_LIMIT 8U
#define PRODUCT_STORAGE_SCHEMA_VERSION 4U
#define PRODUCT_STORAGE_PREVIOUS_VERSION 3U
#define PRODUCT_STORAGE_LEGACY_VERSION 1U
#define PRODUCT_STORAGE_NAMESPACE      "dajilog_v02"
#define PRODUCT_STORAGE_KEY            "product_cfg"
#define RIDGE_PROFILE_STORAGE_KEY       "ridge_list"
#define RIDGE_PROFILE_SCHEMA_VERSION    2U

/* The v1 NVS payload had no next_id; keep its exact layout for migration. */
typedef struct {
    uint32_t count;
    uint32_t selected_id;
    dajilog_ridge_profile_t profiles[DAJILOG_RIDGE_PROFILE_LIMIT];
} ridge_profiles_v1_t;

typedef struct {
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
    dajilog_zone_settings_t zones[DAJILOG_SETTING_ZONE_COUNT];
    dajilog_ridge_settings_t applied_ridge;
    dajilog_prescription_settings_t applied_prescription;
    dajilog_zone_settings_t applied_zones[DAJILOG_SETTING_ZONE_COUNT];
    dajilog_setting_readiness_t saved_readiness;
    uint32_t saved_revision;
    uint32_t applied_revision;
} product_settings_record_t;

typedef struct {
    dajilog_ridge_settings_t ridge;
    dajilog_prescription_settings_t prescription;
    dajilog_setting_readiness_t saved_readiness;
    bool confirmed;
    uint32_t saved_revision;
} product_settings_v3_record_t;

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
static dajilog_zone_settings_t s_zone_draft[DAJILOG_SETTING_ZONE_COUNT];
static dajilog_zone_settings_t s_zone_saved[DAJILOG_SETTING_ZONE_COUNT];
static dajilog_ridge_settings_t s_applied_ridge;
static dajilog_prescription_settings_t s_applied_prescription;
static dajilog_zone_settings_t s_applied_zones[DAJILOG_SETTING_ZONE_COUNT];
static bool s_dirty;
static dajilog_setting_readiness_t s_saved_readiness;
static uint32_t s_draft_generation;
static uint32_t s_saved_revision;
static uint32_t s_applied_revision;
static dajilog_setting_storage_nvs_t s_storage;
static dajilog_ridge_profiles_snapshot_t s_ridge_profiles;
static const char *TAG = "DajilogSettings";

/** @brief Check one ridge's dimensions without accepting example defaults. */
static bool ridge_profile_valid(const dajilog_ridge_profile_input_t *input)
{
    if (input == NULL || !isfinite(input->width_m) ||
        input->width_m <= 0.0f || input->width_m > 100.0f ||
        input->work_width_mode > DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW ||
        input->split_section > 1U)
        return false;
    if (input->work_width_mode == DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW &&
        (!isfinite(input->furrow_width_m) || input->furrow_width_m < 0.0f ||
         input->furrow_width_m > 100.0f))
        return false;
    if (input->split_section) {
        return isfinite(input->north_length_m) && isfinite(input->south_length_m) &&
               input->north_length_m > 0.0f && input->south_length_m > 0.0f &&
               input->north_length_m <= 100000.0f &&
               input->south_length_m <= 100000.0f;
    }
    return isfinite(input->whole_length_m) && input->whole_length_m > 0.0f &&
           input->whole_length_m <= 100000.0f;
}

/** @brief Reject damaged NVS lists before making them visible to the UI. */
static bool ridge_profiles_valid(const dajilog_ridge_profiles_snapshot_t *profiles)
{
    if (profiles->count > DAJILOG_RIDGE_PROFILE_LIMIT ||
        (profiles->count == 0U && profiles->selected_id != 0U) ||
        profiles->next_id == 0U)
        return false;
    for (uint32_t i = 0; i < profiles->count; ++i) {
        if (profiles->profiles[i].id == 0U ||
            profiles->profiles[i].id >= profiles->next_id ||
            (i > 0U && profiles->profiles[i].id <= profiles->profiles[i - 1U].id) ||
            !ridge_profile_valid(&profiles->profiles[i].input))
            return false;
    }
    if (profiles->count == 0U) return true;
    for (uint32_t i = 0; i < profiles->count; ++i) {
        if (profiles->profiles[i].id == profiles->selected_id) return true;
    }
    return false;
}

static bool zone_settings_valid(const dajilog_zone_settings_t *settings)
{
    return settings != NULL &&
           memchr(settings->name, '\0', sizeof(settings->name)) != NULL &&
           memchr(settings->product, '\0', sizeof(settings->product)) != NULL &&
           memchr(settings->lot, '\0', sizeof(settings->lot)) != NULL &&
           isfinite(settings->rate_kg_per_10a) &&
           isfinite(settings->reference_speed_kmh) &&
           settings->rate_kg_per_10a >= 0.0f &&
           settings->rate_kg_per_10a <= 10000.0f &&
           settings->reference_speed_kmh >= 0.0f &&
           settings->reference_speed_kmh <= 50.0f &&
           settings->first_channel_permille <= 1000U;
}

/** @brief Register setting schemas and restore v0.4 or migrate older NVS data. */
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
    dajilog_zone_settings_t initial_zones[DAJILOG_SETTING_ZONE_COUNT] = {0};
    dajilog_ridge_settings_t initial_applied_ridge = {0};
    dajilog_prescription_settings_t initial_applied_prescription = {0};
    dajilog_zone_settings_t initial_applied_zones[DAJILOG_SETTING_ZONE_COUNT] = {0};
    dajilog_setting_readiness_t initial_readiness = DAJILOG_SETTING_UNSET;
    uint32_t initial_saved_revision = 0U;
    uint32_t initial_applied_revision = 0U;
    error = dajilog_setting_storage_nvs_init(
        &s_storage, PRODUCT_STORAGE_NAMESPACE);
    if (error != ESP_OK) {
        return error;
    }
    memset(&s_ridge_profiles, 0, sizeof(s_ridge_profiles));
    s_ridge_profiles.next_id = 1U;
    dajilog_ridge_profiles_snapshot_t restored_profiles = {0};
    const esp_err_t profile_error = dajilog_setting_storage_nvs_load(
        &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
        &restored_profiles, sizeof(restored_profiles));
    if (profile_error == ESP_OK && ridge_profiles_valid(&restored_profiles)) {
        s_ridge_profiles = restored_profiles;
        ESP_LOGI(TAG, "restored %lu independent ridge profiles",
                 (unsigned long)s_ridge_profiles.count);
    } else if (profile_error == ESP_ERR_INVALID_SIZE) {
        ridge_profiles_v1_t legacy = {0};
        const esp_err_t legacy_error = dajilog_setting_storage_nvs_load(
            &s_storage, RIDGE_PROFILE_STORAGE_KEY, 1U, &legacy, sizeof(legacy));
        if (legacy_error == ESP_OK) {
            dajilog_ridge_profiles_snapshot_t migrated = {0};
            migrated.count = legacy.count;
            migrated.selected_id = legacy.selected_id;
            memcpy(migrated.profiles, legacy.profiles, sizeof(legacy.profiles));
            migrated.next_id = legacy.count + 1U;
            if (ridge_profiles_valid(&migrated)) {
                s_ridge_profiles = migrated;
                const esp_err_t migration_error = dajilog_setting_storage_nvs_save(
                    &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
                    &migrated, sizeof(migrated));
                if (migration_error != ESP_OK)
                    ESP_LOGW(TAG, "ridge v1 migration save failed: %s",
                             esp_err_to_name(migration_error));
                ESP_LOGI(TAG, "migrated %lu ridge profiles from v1",
                         (unsigned long)migrated.count);
            } else {
                ESP_LOGW(TAG, "ridge v1 list invalid; migration skipped");
            }
        } else {
            ESP_LOGW(TAG, "ridge v1 migration load failed: %s",
                     esp_err_to_name(legacy_error));
        }
    } else if (profile_error != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "ridge profile list unavailable or invalid: %s",
                 profile_error == ESP_OK ? "invalid contents"
                                         : esp_err_to_name(profile_error));
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
        if (error == ESP_OK && stored.applied_revision != 0U) {
            error = dajilog_table_validate(RIDGE_TABLE_ID,
                                           &stored.applied_ridge,
                                           sizeof(stored.applied_ridge));
            if (error == ESP_OK) {
                error = dajilog_table_validate(PRESCRIPTION_TABLE_ID,
                                               &stored.applied_prescription,
                                               sizeof(stored.applied_prescription));
            }
        }
        if (error == ESP_OK && stored.applied_revision > stored.saved_revision) {
            error = ESP_ERR_INVALID_STATE;
        }
        for (size_t i = 0; error == ESP_OK && i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
            if (!zone_settings_valid(&stored.zones[i]) ||
                !zone_settings_valid(&stored.applied_zones[i])) {
                error = ESP_ERR_INVALID_ARG;
            }
        }
        if (error == ESP_OK) {
            initial_ridge = stored.ridge;
            initial_prescription = stored.prescription;
            memcpy(initial_zones, stored.zones, sizeof(initial_zones));
            initial_applied_ridge = stored.applied_ridge;
            initial_applied_prescription = stored.applied_prescription;
            memcpy(initial_applied_zones, stored.applied_zones,
                   sizeof(initial_applied_zones));
            initial_saved_revision = stored.saved_revision;
            initial_applied_revision = stored.applied_revision;
            initial_readiness = calculate_readiness(
                &initial_ridge, &initial_prescription, false);
            ESP_LOGI(TAG, "restored v0.4 product settings from NVS");
        } else {
            ESP_LOGW(TAG,
                     "stored v0.4 settings failed validation; UNSET used: %s",
                     esp_err_to_name(error));
            error = ESP_OK;
        }
    } else {
        product_settings_v3_record_t previous = {0};
        const esp_err_t previous_error = dajilog_setting_storage_nvs_load(
            &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_PREVIOUS_VERSION,
            &previous, sizeof(previous));
        if (previous_error == ESP_OK &&
            dajilog_table_validate(RIDGE_TABLE_ID, &previous.ridge,
                                   sizeof(previous.ridge)) == ESP_OK &&
            dajilog_table_validate(PRESCRIPTION_TABLE_ID, &previous.prescription,
                                   sizeof(previous.prescription)) == ESP_OK) {
            initial_ridge = previous.ridge;
            initial_prescription = previous.prescription;
            initial_saved_revision = previous.saved_revision;
            initial_readiness = calculate_readiness(
                &initial_ridge, &initial_prescription, false);
            if (previous.confirmed && initial_readiness == DAJILOG_SETTING_CONFIGURED) {
                initial_applied_ridge = previous.ridge;
                initial_applied_prescription = previous.prescription;
                initial_applied_revision = previous.saved_revision;
            }
            ESP_LOGI(TAG, "migrated v0.3 product settings in RAM");
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
    memcpy(s_zone_draft, initial_zones, sizeof(s_zone_draft));
    memcpy(s_zone_saved, initial_zones, sizeof(s_zone_saved));
    s_applied_ridge = initial_applied_ridge;
    s_applied_prescription = initial_applied_prescription;
    memcpy(s_applied_zones, initial_applied_zones, sizeof(s_applied_zones));
    s_saved_readiness = initial_readiness;
    s_saved_revision = initial_saved_revision;
    s_applied_revision = initial_applied_revision;
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
    memcpy(settings->zones, s_zone_draft, sizeof(settings->zones));
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

/** @brief Copy committed ridge profiles under the settings lock. */
esp_err_t dajilog_product_settings_get_ridge_profiles(
    dajilog_ridge_profiles_snapshot_t *profiles)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (profiles == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    *profiles = s_ridge_profiles;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief Commit a new independent Bxx profile before publishing it in RAM. */
esp_err_t dajilog_product_settings_add_ridge_profile(
    const dajilog_ridge_profile_input_t *input)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (!ridge_profile_valid(input)) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_ridge_profiles.count == DAJILOG_RIDGE_PROFILE_LIMIT) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NO_MEM;
    }
    dajilog_ridge_profiles_snapshot_t next = s_ridge_profiles;
    uint32_t id = 1U;
    uint32_t insert_index = 0U;
    while (insert_index < next.count && next.profiles[insert_index].id == id) {
        ++id;
        ++insert_index;
    }
    if (id == next.next_id) {
        if (next.next_id == UINT32_MAX) {
            xSemaphoreGive(s_lock);
            return ESP_ERR_NO_MEM;
        }
        ++next.next_id;
    }
    for (uint32_t i = next.count; i > insert_index; --i)
        next.profiles[i] = next.profiles[i - 1U];
    next.profiles[insert_index].id = id;
    next.profiles[insert_index].input = *input;
    next.count++;
    next.selected_id = id;
    const esp_err_t error = dajilog_setting_storage_nvs_save(
        &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
        &next, sizeof(next));
    if (error == ESP_OK) s_ridge_profiles = next;
    xSemaphoreGive(s_lock);
    return error;
}

/** @brief Commit selection without changing the registered dimensions. */
esp_err_t dajilog_product_settings_select_ridge_profile(uint32_t id)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    bool found = false;
    for (uint32_t i = 0; i < s_ridge_profiles.count; ++i)
        found |= s_ridge_profiles.profiles[i].id == id;
    if (!found) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_ARG;
    }
    dajilog_ridge_profiles_snapshot_t next = s_ridge_profiles;
    next.selected_id = id;
    const esp_err_t error = dajilog_setting_storage_nvs_save(
        &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
        &next, sizeof(next));
    if (error == ESP_OK) s_ridge_profiles = next;
    xSemaphoreGive(s_lock);
    return error;
}

/** @brief Delete a saved Bxx atomically, retaining all other IDs. */
esp_err_t dajilog_product_settings_delete_ridge_profile(uint32_t id)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    uint32_t index = s_ridge_profiles.count;
    for (uint32_t i = 0; i < s_ridge_profiles.count; ++i) {
        if (s_ridge_profiles.profiles[i].id == id) { index = i; break; }
    }
    if (index == s_ridge_profiles.count) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    dajilog_ridge_profiles_snapshot_t next = s_ridge_profiles;
    for (uint32_t i = index + 1U; i < next.count; ++i)
        next.profiles[i - 1U] = next.profiles[i];
    next.count--;
    memset(&next.profiles[next.count], 0, sizeof(next.profiles[next.count]));
    if (next.selected_id == id)
        next.selected_id = next.count == 0U ? 0U :
            next.profiles[index < next.count ? index : next.count - 1U].id;
    const esp_err_t error = dajilog_setting_storage_nvs_save(
        &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
        &next, sizeof(next));
    if (error == ESP_OK) s_ridge_profiles = next;
    xSemaphoreGive(s_lock);
    return error;
}

/** @brief Keep one validated zone draft without changing Saved or Applied. */
esp_err_t dajilog_product_settings_stage_zone(
    dajilog_setting_zone_id_t zone, const dajilog_zone_settings_t *settings)
{
    if (s_lock == NULL || settings == NULL || (unsigned)zone >= DAJILOG_SETTING_ZONE_COUNT)
        return ESP_ERR_INVALID_ARG;
    if (!zone_settings_valid(settings))
        return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_zone_draft[zone] = *settings;
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
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
    ++s_draft_generation;
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
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief Persist Saved inputs while retaining the independent Applied snapshot. */
esp_err_t dajilog_product_settings_save(void)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const dajilog_ridge_settings_t ridge = s_ridge_draft;
    const dajilog_prescription_settings_t prescription =
        s_prescription_draft;
    dajilog_zone_settings_t zones[DAJILOG_SETTING_ZONE_COUNT];
    memcpy(zones, s_zone_draft, sizeof(zones));
    const dajilog_ridge_settings_t applied_ridge = s_applied_ridge;
    const dajilog_prescription_settings_t applied_prescription = s_applied_prescription;
    dajilog_zone_settings_t applied_zones[DAJILOG_SETTING_ZONE_COUNT];
    memcpy(applied_zones, s_applied_zones, sizeof(applied_zones));
    const uint32_t applied_revision = s_applied_revision;
    const uint32_t draft_generation = s_draft_generation;
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
        for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
            const dajilog_zone_settings_t *zone = &zones[i];
            const bool present = zone->name[0] != '\0' || zone->product[0] != '\0' ||
                                 zone->lot[0] != '\0' || zone->rate_kg_per_10a > 0.0f;
            if (present && (!zone->configured || zone->name[0] == '\0' ||
                            zone->product[0] == '\0' ||
                            zone->rate_kg_per_10a <= 0.0f ||
                            zone->reference_speed_kmh <= 0.0f)) {
                error = ESP_ERR_INVALID_STATE;
                break;
            }
        }
    }
    if (error == ESP_OK) {
        const product_settings_record_t record = {
            .ridge = ridge,
            .prescription = prescription,
            .zones = {zones[0], zones[1]},
            .applied_ridge = applied_ridge,
            .applied_prescription = applied_prescription,
            .applied_zones = {applied_zones[0], applied_zones[1]},
            .saved_readiness = readiness,
            .saved_revision = next_revision,
            .applied_revision = applied_revision,
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
        s_dirty = s_draft_generation != draft_generation;
        s_saved_readiness = readiness;
        memcpy(s_zone_saved, zones, sizeof(s_zone_saved));
        s_saved_revision = next_revision;
        xSemaphoreGive(s_lock);
    }
    return error;
}

/** @brief Reject production Apply until approved calibration lookup is installed. */
esp_err_t dajilog_product_settings_confirm(void)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return ESP_ERR_NOT_SUPPORTED;
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
    const bool confirmed = s_applied_revision != 0U;
    if (!confirmed) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    memset(settings, 0, sizeof(*settings));
    settings->ridge = s_applied_ridge;
    settings->prescription = s_applied_prescription;
    memcpy(settings->zones, s_applied_zones, sizeof(settings->zones));
    settings->draft_readiness = DAJILOG_SETTING_CONFIRMED;
    settings->saved_readiness = DAJILOG_SETTING_CONFIRMED;
    settings->saved_revision = s_applied_revision;
    settings->applied_revision = s_applied_revision;
    xSemaphoreGive(s_lock);
    (void)dajilog_table_get_revision(RIDGE_TABLE_ID,
                                     &settings->ridge_revision);
    (void)dajilog_table_get_revision(PRESCRIPTION_TABLE_ID,
                                     &settings->prescription_revision);
    return ESP_OK;
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
        memcpy(s_zone_draft, s_zone_saved, sizeof(s_zone_draft));
        s_dirty = false;
        xSemaphoreGive(s_lock);
    }
    return error;
}
