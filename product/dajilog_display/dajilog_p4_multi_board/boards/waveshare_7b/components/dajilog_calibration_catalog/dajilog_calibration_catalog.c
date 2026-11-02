#include "dajilog_calibration_catalog.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static dajilog_calibration_summary_t s_profiles[DAJILOG_CALIBRATION_MAX_PROFILES];
static dajilog_calibration_point_t s_points[DAJILOG_CALIBRATION_MAX_PROFILES]
                                           [DAJILOG_CALIBRATION_MAX_POINTS];
static size_t s_count;
static SemaphoreHandle_t s_lock;

typedef struct {
    const char *profile_id;
    const char *product_code;
    const char *product_lot;
    char channel;
    unsigned step;
    unsigned decivolt;
    unsigned discharge_g_min;
    bool valid;
    bool measured;
} calibration_csv_row_t;

static const calibration_csv_row_t s_builtin_rows[] = {
#include "dajilog_calibration_data.inc"
};

/** @brief A candidate without measured points or a stable key is unusable. */
static bool valid_candidate(const dajilog_calibration_summary_t *candidate)
{
    return candidate != NULL &&
           memchr(candidate->table_id, '\0', sizeof(candidate->table_id)) != NULL &&
           candidate->table_id[0] != '\0' &&
           memchr(candidate->product_code, '\0', sizeof(candidate->product_code)) != NULL &&
           candidate->product_code[0] != '\0' &&
           memchr(candidate->product_lot, '\0', sizeof(candidate->product_lot)) != NULL &&
           candidate->channel >= 'A' && candidate->channel <= 'D' &&
           candidate->revision != 0U && candidate->content_hash != 0U &&
           candidate->valid_point_count >= 2U &&
           candidate->min_decivolt <= candidate->max_decivolt &&
           candidate->max_decivolt <= 120U &&
           candidate->min_discharge_g_min <= candidate->max_discharge_g_min;
}

esp_err_t dajilog_calibration_catalog_init(void)
{
    if (s_lock != NULL) return ESP_OK;
    s_lock = xSemaphoreCreateMutex();
    return s_lock != NULL ? ESP_OK : ESP_ERR_NO_MEM;
}

/** @brief Load the operator-accepted built-in profiles for calculation preview. */
esp_err_t dajilog_calibration_catalog_load_builtin_csv(void)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (s_count != 0U) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    for (size_t row_index = 0; row_index < sizeof(s_builtin_rows) / sizeof(s_builtin_rows[0]);
         ++row_index) {
        const calibration_csv_row_t *row = &s_builtin_rows[row_index];
        if (!row->valid) continue;
        size_t index = 0U;
        for (; index < s_count; ++index) {
            if (s_profiles[index].channel == row->channel &&
                strcmp(s_profiles[index].table_id, row->profile_id) == 0)
                break;
        }
        if (index == s_count) {
            if (s_count == DAJILOG_CALIBRATION_MAX_PROFILES) goto invalid;
            dajilog_calibration_summary_t *summary = &s_profiles[s_count++];
            memset(summary, 0, sizeof(*summary));
            snprintf(summary->table_id, sizeof(summary->table_id), "%s", row->profile_id);
            snprintf(summary->product_code, sizeof(summary->product_code), "%s", row->product_code);
            snprintf(summary->product_lot, sizeof(summary->product_lot), "%s", row->product_lot);
            summary->channel = row->channel;
            summary->revision = 1U;
            summary->content_hash = 2166136261U;
            summary->measured = row->measured;
            summary->approved = true;
            summary->min_decivolt = (uint16_t)row->decivolt;
            summary->min_discharge_g_min = row->discharge_g_min;
        }
        dajilog_calibration_summary_t *summary = &s_profiles[index];
        if (strcmp(summary->product_code, row->product_code) != 0 ||
            strcmp(summary->product_lot, row->product_lot) != 0 ||
            summary->measured != row->measured ||
            summary->valid_point_count == DAJILOG_CALIBRATION_MAX_POINTS)
            goto invalid;
        if (summary->valid_point_count != 0U) {
            const dajilog_calibration_point_t *previous =
                &s_points[index][summary->valid_point_count - 1U];
            if (row->step <= previous->command_step ||
                row->decivolt <= previous->command_decivolt ||
                row->discharge_g_min < previous->discharge_g_min)
                goto invalid;
        }
        s_points[index][summary->valid_point_count++] = (dajilog_calibration_point_t){
            .command_step = (uint8_t)row->step,
            .command_decivolt = (uint16_t)row->decivolt,
            .discharge_g_min = row->discharge_g_min,
        };
        summary->max_decivolt = (uint16_t)row->decivolt;
        summary->max_discharge_g_min = row->discharge_g_min;
        summary->content_hash = (summary->content_hash ^ row->step) * 16777619U;
        summary->content_hash = (summary->content_hash ^ row->decivolt) * 16777619U;
        summary->content_hash = (summary->content_hash ^ row->discharge_g_min) * 16777619U;
    }
    for (size_t i = 0; i < s_count; ++i) {
        if (!valid_candidate(&s_profiles[i])) goto invalid;
    }
    xSemaphoreGive(s_lock);
    return ESP_OK;

invalid:
    memset(s_profiles, 0, sizeof(s_profiles));
    memset(s_points, 0, sizeof(s_points));
    s_count = 0U;
    xSemaphoreGive(s_lock);
    return ESP_ERR_INVALID_ARG;
}

esp_err_t dajilog_calibration_catalog_load_test_samples(void)
{
    /* Mirrors the four channel summaries in config/examples. No CSV is parsed
     * here and no measured point data or approval is installed. */
    static const struct {
        char channel;
        uint32_t min_g_min;
        uint32_t max_g_min;
    } samples[] = {
        {'A', 100U, 390U}, {'B', 95U, 376U},
        {'C', 108U, 410U}, {'D', 102U, 397U},
    };
    for (size_t i = 0; i < sizeof(samples) / sizeof(samples[0]); ++i) {
        dajilog_calibration_summary_t candidate = {0};
        snprintf(candidate.table_id, sizeof(candidate.table_id),
                 "SAMPLE_ONLY_%c", samples[i].channel);
        memcpy(candidate.product_code, "DEMO_ONLY", sizeof("DEMO_ONLY"));
        memcpy(candidate.product_lot, "SYNTHETIC", sizeof("SYNTHETIC"));
        candidate.channel = samples[i].channel;
        candidate.revision = 1U;
        candidate.content_hash = 0x53414D00U + (uint32_t)samples[i].channel;
        candidate.min_decivolt = 30U;
        candidate.max_decivolt = 60U;
        candidate.min_discharge_g_min = samples[i].min_g_min;
        candidate.max_discharge_g_min = samples[i].max_g_min;
        candidate.valid_point_count = 4U;
        candidate.approved = false;
        esp_err_t err = dajilog_calibration_catalog_add_candidate(&candidate);
        if (err != ESP_OK) return err;
    }
    return ESP_OK;
}

esp_err_t dajilog_calibration_catalog_add_candidate(
    const dajilog_calibration_summary_t *candidate)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (!valid_candidate(candidate) || candidate->approved) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t i = 0; i < s_count; ++i) {
        if (s_profiles[i].channel == candidate->channel &&
            strcmp(s_profiles[i].table_id, candidate->table_id) == 0 &&
            s_profiles[i].revision == candidate->revision) {
            xSemaphoreGive(s_lock);
            return ESP_ERR_INVALID_STATE;
        }
    }
    if (s_count == DAJILOG_CALIBRATION_MAX_PROFILES) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NO_MEM;
    }
    s_profiles[s_count++] = *candidate;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

size_t dajilog_calibration_catalog_list(
    char channel, const char *product_code, const char *product_lot,
    dajilog_calibration_summary_t *out, size_t capacity)
{
    if (s_lock == NULL || channel < 'A' || channel > 'D' ||
        (capacity != 0U && out == NULL)) return 0U;
    size_t matched = 0U;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t i = 0; i < s_count; ++i) {
        const dajilog_calibration_summary_t *profile = &s_profiles[i];
        if (profile->channel != channel ||
            (product_code != NULL && product_code[0] != '\0' &&
             strcmp(profile->product_code, product_code) != 0) ||
            (product_lot != NULL && product_lot[0] != '\0' &&
             strcmp(profile->product_lot, product_lot) != 0)) continue;
        if (matched < capacity) out[matched] = *profile;
        ++matched;
    }
    xSemaphoreGive(s_lock);
    return matched;
}

esp_err_t dajilog_calibration_catalog_find(
    char channel, const char *table_id, uint32_t revision,
    dajilog_calibration_summary_t *out)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (channel < 'A' || channel > 'D' || table_id == NULL || out == NULL)
        return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t i = 0; i < s_count; ++i) {
        if (s_profiles[i].channel == channel &&
            s_profiles[i].revision == revision &&
            strcmp(s_profiles[i].table_id, table_id) == 0) {
            *out = s_profiles[i];
            xSemaphoreGive(s_lock);
            return ESP_OK;
        }
    }
    xSemaphoreGive(s_lock);
    return ESP_ERR_NOT_FOUND;
}

/** @brief Return the stored CSV points for a channel and stable table ID. */
esp_err_t dajilog_calibration_catalog_get_points(
    char channel, const char *table_id, uint32_t revision,
    dajilog_calibration_point_t *out, size_t capacity, size_t *out_count)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (channel < 'A' || channel > 'D' || table_id == NULL || out_count == NULL ||
        (capacity != 0U && out == NULL)) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t i = 0; i < s_count; ++i) {
        if (s_profiles[i].channel == channel && s_profiles[i].revision == revision &&
            strcmp(s_profiles[i].table_id, table_id) == 0) {
            const size_t count = s_profiles[i].valid_point_count;
            if (capacity < count) {
                xSemaphoreGive(s_lock);
                return ESP_ERR_INVALID_SIZE;
            }
            memcpy(out, s_points[i], count * sizeof(s_points[i][0]));
            *out_count = count;
            xSemaphoreGive(s_lock);
            return ESP_OK;
        }
    }
    xSemaphoreGive(s_lock);
    return ESP_ERR_NOT_FOUND;
}

/** @brief Invert one monotonic discharge table without extrapolation or CAN I/O. */
esp_err_t dajilog_calibration_catalog_preview_voltage(
    char channel, const char *table_id, uint32_t revision, uint32_t target_g_min,
    uint16_t *out_decivolt, uint32_t *out_expected_g_min)
{
    if (out_decivolt == NULL || out_expected_g_min == NULL) return ESP_ERR_INVALID_ARG;
    if (target_g_min == 0U) {
        *out_decivolt = 0U;
        *out_expected_g_min = 0U;
        return ESP_OK;
    }
    dajilog_calibration_point_t points[DAJILOG_CALIBRATION_MAX_POINTS];
    size_t count = 0U;
    esp_err_t err = dajilog_calibration_catalog_get_points(
        channel, table_id, revision, points, DAJILOG_CALIBRATION_MAX_POINTS, &count);
    if (err != ESP_OK) return err;
    if (count < 2U || target_g_min < points[0].discharge_g_min ||
        target_g_min > points[count - 1U].discharge_g_min) return ESP_ERR_INVALID_ARG;
    for (size_t i = 1U; i < count; ++i) {
        if (target_g_min > points[i].discharge_g_min) continue;
        const uint32_t low_g = points[i - 1U].discharge_g_min;
        const uint32_t high_g = points[i].discharge_g_min;
        const uint32_t low_v = points[i - 1U].command_decivolt;
        const uint32_t high_v = points[i].command_decivolt;
        if (high_g == low_g) return ESP_ERR_INVALID_ARG;
        const uint32_t delta_v = ((target_g_min - low_g) * (high_v - low_v) +
                                  (high_g - low_g) / 2U) / (high_g - low_g);
        *out_decivolt = (uint16_t)(low_v + delta_v);
        *out_expected_g_min = low_g +
            (delta_v * (high_g - low_g) + (high_v - low_v) / 2U) / (high_v - low_v);
        return ESP_OK;
    }
    return ESP_ERR_INVALID_ARG;
}
