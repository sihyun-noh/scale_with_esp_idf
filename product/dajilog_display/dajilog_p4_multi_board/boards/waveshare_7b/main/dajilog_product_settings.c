/**
 * @file dajilog_product_settings.c
 * @brief 7B 제품 설정의 RAM Draft, Saved/Applied, Bxx 목록 및 NVS 저장소.
 *
 * 찾을 때: Bxx 추가·선택·삭제=*_ridge_profile(), 두둑 카드 임시저장=
 * stage_ridge_profile(), 처방 카드 임시저장=stage_prescription_products(),
 * 설정 저장=save(), 재부팅 복원=init(). LVGL 화면 객체는 여기서 다루지 않는다.
 */
#include "dajilog_product_settings.h"

#include <stddef.h>
#include <string.h>
#include <math.h>

#include "dajilog_table.h"
#include "dajilog_setting_storage_nvs.h"
#include "dajilog_product_catalog.h"
#include "dajilog_calibration_catalog.h"
#include "dajilog_prescription_calculator.h"
#include "esp_log.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define RIDGE_TABLE_ID        0x07030101U
#define PRESCRIPTION_TABLE_ID 0x07030102U
#define PRODUCT_PROFILE_LIMIT 8U
#define PRODUCT_STORAGE_SCHEMA_VERSION 6U
#define PRODUCT_STORAGE_V5_VERSION       5U
#define PRODUCT_STORAGE_V4_VERSION       4U
#define PRODUCT_STORAGE_PREVIOUS_VERSION 3U
#define PRODUCT_STORAGE_LEGACY_VERSION 1U
#define PRODUCT_STORAGE_NAMESPACE      "dajilog_v02"
#define PRODUCT_STORAGE_KEY            "product_cfg"
#define RIDGE_PROFILE_STORAGE_KEY       "ridge_list"
#define RIDGE_PROFILE_SCHEMA_VERSION    2U

/* [저장소 호환] v1 NVS에는 next_id가 없으므로 이전 레이아웃을 유지해 이전한다. */
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
} product_settings_v4_record_t;

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
    char selected_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
} product_settings_v5_record_t;

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
    char selected_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
    dajilog_calibration_binding_t selected_calibration[DAJILOG_MOTOR_CHANNEL_COUNT];
    dajilog_calibration_binding_t applied_calibration[DAJILOG_MOTOR_CHANNEL_COUNT];
    char applied_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
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

static dajilog_ridge_profiles_snapshot_t s_ridge_profiles;
static bool ridge_profile_valid(const dajilog_ridge_profile_input_t *input);

static dajilog_setting_readiness_t calculate_readiness(
    const dajilog_ridge_settings_t *ridge,
    const dajilog_prescription_settings_t *prescription,
    const char codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY],
    bool confirmed)
{
    for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
        if (dajilog_product_catalog_find(codes[i]) == NULL)
            return DAJILOG_SETTING_UNSET;
    }
    bool selected_ridge_ready = false;
    for (uint32_t i = 0; i < s_ridge_profiles.count; ++i) {
        if (s_ridge_profiles.profiles[i].id == s_ridge_profiles.selected_id &&
            ridge_profile_valid(&s_ridge_profiles.profiles[i].input)) {
            selected_ridge_ready = true;
            break;
        }
    }
    const bool legacy_ridge_ready =
        (ridge->presence_mask & DAJILOG_RIDGE_REQUIRED_MASK) ==
            DAJILOG_RIDGE_REQUIRED_MASK &&
        (ridge->work_width_mode != DAJILOG_WORK_WIDTH_RIDGE_AND_FURROW ||
         (ridge->presence_mask & DAJILOG_RIDGE_PRESENT_FURROW_WIDTH) != 0U);
    if (!selected_ridge_ready && !legacy_ridge_ready)
        return DAJILOG_SETTING_UNSET;

    uint32_t required = DAJILOG_PRESCRIPTION_PRESENT_INPUT_MODE;
    if (prescription->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN) {
        required |= DAJILOG_PRESCRIPTION_PRESENT_SURFACE_RATIO |
                    DAJILOG_PRESCRIPTION_PRESENT_NITROGEN_TARGET |
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
static char s_product_draft[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
static char s_product_saved[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
static dajilog_calibration_binding_t s_calibration_draft[DAJILOG_MOTOR_CHANNEL_COUNT];
static dajilog_calibration_binding_t s_calibration_saved[DAJILOG_MOTOR_CHANNEL_COUNT];
static dajilog_calibration_binding_t s_calibration_applied[DAJILOG_MOTOR_CHANNEL_COUNT];
static char s_product_applied[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
static dajilog_ridge_settings_t s_applied_ridge;
static dajilog_prescription_settings_t s_applied_prescription;
static dajilog_zone_settings_t s_applied_zones[DAJILOG_SETTING_ZONE_COUNT];
static bool s_dirty;
static dajilog_setting_readiness_t s_saved_readiness;
static uint32_t s_draft_generation;
static uint32_t s_saved_revision;
static uint32_t s_applied_revision;
static dajilog_setting_storage_nvs_t s_storage;

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
    /* A newly registered Bxx may have no length until RidgeSetup saves it. */
    return isfinite(input->whole_length_m) && input->whole_length_m >= 0.0f &&
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

/** @brief [처방→S1/S2] 구역 제품량의 단일 출처를 처방 계산 결과로 맞춘다. */
static void derive_zone_product_rates(
    const dajilog_prescription_settings_t *prescription,
    const char codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY],
    dajilog_zone_settings_t zones[DAJILOG_SETTING_ZONE_COUNT])
{
    dajilog_prescription_product_rates_t rates = {0};
    const dajilog_prescription_calc_status_t status =
        dajilog_prescription_calculate_product_rates(prescription, codes, &rates);
    zones[DAJILOG_SETTING_ZONE_SURFACE].rate_kg_per_10a =
        status == DAJILOG_PRESCRIPTION_CALC_OK ? rates.surface_product_kg_per_10a : 0.0f;
    zones[DAJILOG_SETTING_ZONE_SUBSURFACE].rate_kg_per_10a =
        status == DAJILOG_PRESCRIPTION_CALC_OK ? rates.subsurface_product_kg_per_10a : 0.0f;
}

/** @brief [부팅·저장소] 스키마를 등록하고 NVS의 Product 설정과 Bxx 목록을 복원한다. */
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
    char initial_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY] = {0};
    dajilog_calibration_binding_t initial_calibration[DAJILOG_MOTOR_CHANNEL_COUNT] = {0};
    dajilog_calibration_binding_t initial_applied_calibration[DAJILOG_MOTOR_CHANNEL_COUNT] = {0};
    char initial_applied_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY] = {0};
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
    if (error != ESP_OK) {
        product_settings_v5_record_t previous_v5 = {0};
        if (dajilog_setting_storage_nvs_load(
                &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_V5_VERSION,
                &previous_v5, sizeof(previous_v5)) == ESP_OK) {
            memcpy(&stored, &previous_v5, sizeof(previous_v5));
            stored.applied_revision = 0U;
            error = ESP_OK;
            ESP_LOGI(TAG, "migrating v0.5 settings; calibration selection required");
        }
    }
    if (error != ESP_OK) {
        product_settings_v4_record_t previous_v4 = {0};
        const esp_err_t previous_v4_error = dajilog_setting_storage_nvs_load(
            &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_V4_VERSION,
            &previous_v4, sizeof(previous_v4));
        if (previous_v4_error == ESP_OK) {
            memcpy(&stored, &previous_v4, sizeof(previous_v4));
            stored.applied_revision = 0U;
            error = ESP_OK;
            ESP_LOGI(TAG, "migrating v0.4 product settings; products remain unselected");
        }
    }
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
                !zone_settings_valid(&stored.applied_zones[i]) ||
                memchr(stored.selected_product_codes[i], '\0',
                       sizeof(stored.selected_product_codes[i])) == NULL) {
                error = ESP_ERR_INVALID_ARG;
            }
        }
        if (error == ESP_OK) {
            initial_ridge = stored.ridge;
            initial_prescription = stored.prescription;
            memcpy(initial_zones, stored.zones, sizeof(initial_zones));
            memcpy(initial_product_codes, stored.selected_product_codes,
                   sizeof(initial_product_codes));
            memcpy(initial_calibration, stored.selected_calibration, sizeof(initial_calibration));
            memcpy(initial_applied_calibration, stored.applied_calibration, sizeof(initial_applied_calibration));
            memcpy(initial_applied_product_codes, stored.applied_product_codes, sizeof(initial_applied_product_codes));
            initial_applied_ridge = stored.applied_ridge;
            initial_applied_prescription = stored.applied_prescription;
            memcpy(initial_applied_zones, stored.applied_zones,
                   sizeof(initial_applied_zones));
            initial_saved_revision = stored.saved_revision;
            initial_applied_revision = stored.applied_revision;
            initial_readiness = calculate_readiness(
                &initial_ridge, &initial_prescription, initial_product_codes, false);
            ESP_LOGI(TAG, "restored product settings from NVS");
        } else {
            ESP_LOGW(TAG,
                     "stored product settings failed validation; UNSET used: %s",
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
                &initial_ridge, &initial_prescription, initial_product_codes, false);
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
    memcpy(s_product_draft, initial_product_codes, sizeof(s_product_draft));
    memcpy(s_product_saved, initial_product_codes, sizeof(s_product_saved));
    memcpy(s_calibration_draft, initial_calibration, sizeof(s_calibration_draft));
    memcpy(s_calibration_saved, initial_calibration, sizeof(s_calibration_saved));
    memcpy(s_calibration_applied, initial_applied_calibration, sizeof(s_calibration_applied));
    memcpy(s_product_applied, initial_applied_product_codes, sizeof(s_product_applied));
    s_applied_ridge = initial_applied_ridge;
    s_applied_prescription = initial_applied_prescription;
    memcpy(s_applied_zones, initial_applied_zones, sizeof(s_applied_zones));
    s_saved_readiness = initial_readiness;
    s_saved_revision = initial_saved_revision;
    s_applied_revision = initial_applied_revision;
    s_lock = lock;
    return ESP_OK;
}

/** @brief [설정 카드 공통] RAM의 편집 중 Draft와 Table revision을 복사한다. */
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
    memcpy(settings->selected_product_codes, s_product_draft,
           sizeof(settings->selected_product_codes));
    derive_zone_product_rates(&settings->prescription,
                              settings->selected_product_codes, settings->zones);
    memcpy(settings->calibration, s_calibration_draft, sizeof(settings->calibration));
    settings->dirty = s_dirty;
    settings->draft_readiness = calculate_readiness(
        &s_ridge_draft, &s_prescription_draft, s_product_draft, false);
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

/** @brief [두둑 카드] 잠금 아래에서 현재 Bxx RAM 목록과 선택 ID를 복사한다. */
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

/** @brief [두둑 설정 저장] 다른 처방·구역 Draft와 독립적으로 Bxx 목록을 NVS에 보존한다. */
esp_err_t dajilog_product_settings_save_ridge_profiles(void)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    dajilog_ridge_profiles_snapshot_t profiles = {0};
    xSemaphoreTake(s_lock, portMAX_DELAY);
    profiles = s_ridge_profiles;
    xSemaphoreGive(s_lock);
    if (!ridge_profiles_valid(&profiles)) return ESP_ERR_INVALID_STATE;
    return dajilog_setting_storage_nvs_save(
        &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
        &profiles, sizeof(profiles));
}

/** @brief [두둑 카드·새 두둑 추가] 빈 B번호에 폭만 등록하고 길이는 이후 설정한다. */
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
    s_ridge_profiles = next;
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [두둑 카드·두둑 변경] 선택 Bxx의 폭만 수정하고 구간 길이는 유지한다. */
esp_err_t dajilog_product_settings_edit_ridge_profile(
    const dajilog_ridge_profile_edit_t *edit)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (edit == NULL || edit->id == 0U || !isfinite(edit->width_m) ||
        edit->width_m <= 0.0f || edit->width_m > 100.0f) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (uint32_t i = 0; i < s_ridge_profiles.count; ++i) {
        if (s_ridge_profiles.profiles[i].id != edit->id) continue;
        dajilog_ridge_profile_input_t *input = &s_ridge_profiles.profiles[i].input;
        input->width_m = edit->width_m;
        s_dirty = true;
        ++s_draft_generation;
        xSemaphoreGive(s_lock);
        return ESP_OK;
    }
    xSemaphoreGive(s_lock);
    return ESP_ERR_NOT_FOUND;
}

/** @brief [두둑 카드·두둑 선택] 선택 ID만 RAM에서 바꾸고 NVS는 쓰지 않는다. */
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
    s_ridge_profiles.selected_id = id;
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [두둑 카드·삭제] 선택 Bxx를 RAM 목록에서 제거하고 다른 ID는 유지한다. */
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
    s_ridge_profiles = next;
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [설정 카드 S1/S2] 구역 입력을 검증해 RAM Draft만 변경한다. */
esp_err_t dajilog_product_settings_stage_zone(
    dajilog_setting_zone_id_t zone, const dajilog_zone_settings_t *settings)
{
    if (s_lock == NULL || settings == NULL || (unsigned)zone >= DAJILOG_SETTING_ZONE_COUNT)
        return ESP_ERR_INVALID_ARG;
    if (!zone_settings_valid(settings))
        return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_zone_draft[zone] = *settings;
    derive_zone_product_rates(&s_prescription_draft, s_product_draft, s_zone_draft);
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [두둑 카드 공통] 계산용 단일 Ridge Draft를 검증해 RAM에서 교체한다. */
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

/** @brief [두둑 카드 임시저장] 선택 Bxx 상세값과 계산용 Draft를 한 잠금에서 함께 교체한다. */
esp_err_t dajilog_product_settings_stage_ridge_profile(
    uint32_t id, const dajilog_ridge_profile_input_t *profile,
    const dajilog_ridge_settings_t *settings)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (id == 0U || !ridge_profile_valid(profile) || settings == NULL) return ESP_ERR_INVALID_ARG;
    const esp_err_t error = dajilog_table_validate(
        RIDGE_TABLE_ID, settings, sizeof(*settings));
    if (error != ESP_OK) return error;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_ridge_profiles.selected_id != id) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    for (uint32_t i = 0; i < s_ridge_profiles.count; ++i) {
        if (s_ridge_profiles.profiles[i].id != id) continue;
        s_ridge_profiles.profiles[i].input = *profile;
        s_ridge_draft = *settings;
        s_dirty = true;
        ++s_draft_generation;
        xSemaphoreGive(s_lock);
        return ESP_OK;
    }
    xSemaphoreGive(s_lock);
    return ESP_ERR_NOT_FOUND;
}

/** @brief [처방 카드] 처방 입력을 검증해 RAM Draft만 변경한다. */
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

/** @brief [처방 카드] 처방값과 선택 제품 코드를 RAM Draft에 함께 반영한다. */
esp_err_t dajilog_product_settings_stage_prescription_products(
    const dajilog_prescription_settings_t *settings,
    const char codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY])
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (settings == NULL || codes == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t error = dajilog_table_validate(
        PRESCRIPTION_TABLE_ID, settings, sizeof(*settings));
    if (error != ESP_OK) return error;
    for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
        if (memchr(codes[i], '\0', DAJILOG_PRODUCT_ID_CAPACITY) == NULL)
            return ESP_ERR_INVALID_ARG;
        const dajilog_product_entry_t *product = dajilog_product_catalog_find(codes[i]);
        if (codes[i][0] != '\0' && product == NULL) return ESP_ERR_NOT_FOUND;
        if (settings->input_mode == DAJILOG_PRESCRIPTION_BY_NITROGEN && product != NULL) {
            const float actual = i == DAJILOG_SETTING_ZONE_SURFACE
                ? settings->surface_fertilizer_nitrogen_pct
                : settings->subsurface_fertilizer_nitrogen_pct;
            if (fabsf(actual - product->nitrogen_percent) > 0.001f)
                return ESP_ERR_INVALID_ARG;
        }
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_prescription_draft = *settings;
    memcpy(s_product_draft, codes, sizeof(s_product_draft));
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [배출량표 조회] 마지막 NVS 저장 성공 값만 Saved로 반환한다. */
esp_err_t dajilog_product_settings_get_saved(
    dajilog_product_settings_snapshot_t *settings)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (settings == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    memset(settings, 0, sizeof(*settings));
    memcpy(settings->zones, s_zone_saved, sizeof(settings->zones));
    memcpy(settings->selected_product_codes, s_product_saved,
           sizeof(settings->selected_product_codes));
    memcpy(settings->calibration, s_calibration_saved, sizeof(settings->calibration));
    settings->saved_readiness = s_saved_readiness;
    settings->saved_revision = s_saved_revision;
    settings->applied_revision = s_applied_revision;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [설정 저장 버튼] Product Draft와 전체 Bxx 목록을 NVS에 저장하고 Applied는 유지한다. */
esp_err_t dajilog_product_settings_save(void)
{
    if (s_lock == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const dajilog_ridge_settings_t ridge = s_ridge_draft;
    const dajilog_ridge_profiles_snapshot_t ridge_profiles = s_ridge_profiles;
    const dajilog_prescription_settings_t prescription =
        s_prescription_draft;
    dajilog_zone_settings_t zones[DAJILOG_SETTING_ZONE_COUNT];
    memcpy(zones, s_zone_draft, sizeof(zones));
    char product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
    memcpy(product_codes, s_product_draft, sizeof(product_codes));
    dajilog_calibration_binding_t calibration[DAJILOG_MOTOR_CHANNEL_COUNT];
    dajilog_calibration_binding_t applied_calibration[DAJILOG_MOTOR_CHANNEL_COUNT];
    memcpy(calibration, s_calibration_draft, sizeof(calibration));
    memcpy(applied_calibration, s_calibration_applied, sizeof(applied_calibration));
    char applied_product_codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
    memcpy(applied_product_codes, s_product_applied, sizeof(applied_product_codes));
    derive_zone_product_rates(&prescription, product_codes, zones);
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
        calculate_readiness(&ridge, &prescription, product_codes, false);
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const uint32_t current_revision = s_saved_revision;
    xSemaphoreGive(s_lock);
    const uint32_t incremented_revision = current_revision + 1U;
    const uint32_t next_revision =
        incremented_revision == 0U ? 1U : incremented_revision;
    if (error == ESP_OK) {
        for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
            const dajilog_zone_settings_t *zone = &zones[i];
            /* Save each zone independently, including an incomplete Draft.
             * Readiness/Apply still require complete settings. */
            if (!zone_settings_valid(zone)) {
                error = ESP_ERR_INVALID_STATE;
                break;
            }
        }
    }
    if (error == ESP_OK) {
        product_settings_record_t record = {
            .ridge = ridge,
            .prescription = prescription,
            .zones = {zones[0], zones[1]},
            .applied_ridge = applied_ridge,
            .applied_prescription = applied_prescription,
            .applied_zones = {applied_zones[0], applied_zones[1]},
            .saved_readiness = readiness,
            .saved_revision = next_revision,
            .applied_revision = applied_revision,
            .selected_product_codes = {{0}},
        };
        memcpy(record.selected_product_codes, product_codes,
               sizeof(product_codes));
        memcpy(record.selected_calibration, calibration, sizeof(calibration));
        memcpy(record.applied_calibration, applied_calibration, sizeof(applied_calibration));
        memcpy(record.applied_product_codes, applied_product_codes, sizeof(applied_product_codes));
        error = dajilog_setting_storage_nvs_save(
            &s_storage, PRODUCT_STORAGE_KEY,
            PRODUCT_STORAGE_SCHEMA_VERSION, &record, sizeof(record));
    }
    if (error == ESP_OK) {
        error = dajilog_setting_storage_nvs_save(
            &s_storage, RIDGE_PROFILE_STORAGE_KEY, RIDGE_PROFILE_SCHEMA_VERSION,
            &ridge_profiles, sizeof(ridge_profiles));
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
        memcpy(s_product_saved, product_codes, sizeof(s_product_saved));
        memcpy(s_calibration_saved, calibration, sizeof(s_calibration_saved));
        s_saved_revision = next_revision;
        xSemaphoreGive(s_lock);
    }
    return error;
}

/** @brief [배출량표 선택] 표의 ID, revision, hash를 해당 채널 Draft에 고정한다. */
esp_err_t dajilog_product_settings_stage_calibration(
    char channel, const dajilog_calibration_binding_t *binding)
{
    if (s_lock == NULL || channel < 'A' || channel > 'D' || binding == NULL ||
        memchr(binding->table_id, '\0', sizeof(binding->table_id)) == NULL ||
        binding->table_id[0] == '\0' || binding->revision == 0U ||
        binding->content_hash == 0U) return ESP_ERR_INVALID_ARG;
    dajilog_calibration_summary_t table = {0};
    esp_err_t error = dajilog_calibration_catalog_find(
        channel, binding->table_id, binding->revision, &table);
    if (error != ESP_OK) return error;
    if (!table.approved || table.content_hash != binding->content_hash)
        return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    const size_t zone_index = (size_t)(channel - 'A') / 2U;
    if (strcmp(table.product_code, s_product_draft[zone_index]) != 0 ||
        (s_zone_draft[zone_index].lot[0] != '\0' &&
         strcmp(table.product_lot, s_zone_draft[zone_index].lot) != 0)) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    s_calibration_draft[channel - 'A'] = *binding;
    s_dirty = true;
    ++s_draft_generation;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [설정 카드 Confirm] Saved와 4채널 승인 표를 검증한 뒤 Applied를 NVS에 기록한다. */
esp_err_t dajilog_product_settings_confirm(void)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    product_settings_record_t current = {0};
    const bool ready = !s_dirty && s_saved_revision != 0U &&
        s_saved_readiness == DAJILOG_SETTING_CONFIGURED &&
        s_ridge_profiles.selected_id != 0U;
    const uint32_t revision = s_saved_revision;
    dajilog_calibration_binding_t selected[DAJILOG_MOTOR_CHANNEL_COUNT];
    memcpy(selected, s_calibration_saved, sizeof(selected));
    dajilog_zone_settings_t zones[DAJILOG_SETTING_ZONE_COUNT];
    memcpy(zones, s_zone_saved, sizeof(zones));
    char codes[DAJILOG_SETTING_ZONE_COUNT][DAJILOG_PRODUCT_ID_CAPACITY];
    memcpy(codes, s_product_saved, sizeof(codes));
    const dajilog_ridge_settings_t ridge = s_ridge_draft;
    const dajilog_prescription_settings_t prescription = s_prescription_draft;
    xSemaphoreGive(s_lock);
    if (!ready) return ESP_ERR_INVALID_STATE;
    for (size_t i = 0; i < DAJILOG_SETTING_ZONE_COUNT; ++i) {
        if (!zones[i].configured || zones[i].name[0] == '\0' ||
            zones[i].reference_speed_kmh <= 0.0f ||
            zones[i].rate_kg_per_10a <= 0.0f || codes[i][0] == '\0')
            return ESP_ERR_INVALID_STATE;
    }
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        const dajilog_calibration_binding_t *binding = &selected[i];
        dajilog_calibration_summary_t table = {0};
        if (binding->table_id[0] == '\0' || binding->revision == 0U ||
            dajilog_calibration_catalog_find((char)('A' + i), binding->table_id,
                                             binding->revision, &table) != ESP_OK ||
            !table.approved || table.content_hash != binding->content_hash ||
            strcmp(table.product_code, codes[i / 2U]) != 0 ||
            (zones[i / 2U].lot[0] != '\0' &&
             strcmp(table.product_lot, zones[i / 2U].lot) != 0))
            return ESP_ERR_INVALID_STATE;
    }
    esp_err_t error = dajilog_setting_storage_nvs_load(
        &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_SCHEMA_VERSION,
        &current, sizeof(current));
    if (error != ESP_OK) return error;
    if (current.saved_revision != revision ||
        memcmp(current.selected_calibration, selected, sizeof(selected)) != 0)
        return ESP_ERR_INVALID_STATE;
    current.applied_ridge = ridge;
    current.applied_prescription = prescription;
    memcpy(current.applied_zones, zones, sizeof(zones));
    memcpy(current.applied_calibration, selected, sizeof(selected));
    memcpy(current.applied_product_codes, codes, sizeof(codes));
    current.applied_revision = revision;
    error = dajilog_setting_storage_nvs_save(
        &s_storage, PRODUCT_STORAGE_KEY, PRODUCT_STORAGE_SCHEMA_VERSION,
        &current, sizeof(current));
    if (error != ESP_OK) return error;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_applied_ridge = ridge;
    s_applied_prescription = prescription;
    memcpy(s_applied_zones, zones, sizeof(zones));
    memcpy(s_calibration_applied, selected, sizeof(selected));
    memcpy(s_product_applied, codes, sizeof(codes));
    s_applied_revision = revision;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

/** @brief [Dashboard RUN] 확인된 Applied 설정만 운전 시작 측에 제공한다. */
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
    memcpy(settings->calibration, s_calibration_applied, sizeof(settings->calibration));
    memcpy(settings->selected_product_codes, s_product_applied, sizeof(settings->selected_product_codes));
    settings->draft_readiness = DAJILOG_SETTING_CONFIRMED;
    settings->saved_readiness = DAJILOG_SETTING_CONFIRMED;
    settings->saved_revision = s_applied_revision;
    settings->applied_revision = s_applied_revision;
    xSemaphoreGive(s_lock);
    for (size_t i = 0; i < DAJILOG_MOTOR_CHANNEL_COUNT; ++i) {
        const dajilog_calibration_binding_t *binding = &settings->calibration[i];
        dajilog_calibration_summary_t table = {0};
        if (binding->table_id[0] == '\0' ||
            dajilog_calibration_catalog_find((char)('A' + i), binding->table_id,
                                             binding->revision, &table) != ESP_OK ||
            !table.approved || table.content_hash != binding->content_hash ||
            strcmp(table.product_code, settings->selected_product_codes[i / 2U]) != 0)
            return ESP_ERR_INVALID_STATE;
    }
    (void)dajilog_table_get_revision(RIDGE_TABLE_ID,
                                     &settings->ridge_revision);
    (void)dajilog_table_get_revision(PRESCRIPTION_TABLE_ID,
                                     &settings->prescription_revision);
    return ESP_OK;
}

/** @brief [설정 취소] Saved Table Row에서 편집 중 Draft를 다시 불러온다. */
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
        memcpy(s_product_draft, s_product_saved, sizeof(s_product_draft));
        s_dirty = false;
        xSemaphoreGive(s_lock);
    }
    return error;
}
