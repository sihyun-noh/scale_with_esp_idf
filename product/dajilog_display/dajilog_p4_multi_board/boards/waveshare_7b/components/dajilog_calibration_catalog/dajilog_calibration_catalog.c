#include "dajilog_calibration_catalog.h"

#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static dajilog_calibration_summary_t s_profiles[DAJILOG_CALIBRATION_MAX_PROFILES];
static size_t s_count;
static SemaphoreHandle_t s_lock;

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
