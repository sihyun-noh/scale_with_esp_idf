#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_CALIBRATION_ID_SIZE 40U
#define DAJILOG_CALIBRATION_PRODUCT_SIZE 32U
#define DAJILOG_CALIBRATION_LOT_SIZE 32U
#define DAJILOG_CALIBRATION_MAX_PROFILES 16U
#define DAJILOG_CALIBRATION_MAX_POINTS 24U

typedef struct {
    uint8_t command_step;
    uint16_t command_decivolt;
    uint32_t discharge_g_min;
} dajilog_calibration_point_t;

typedef struct {
    char table_id[DAJILOG_CALIBRATION_ID_SIZE];
    char product_code[DAJILOG_CALIBRATION_PRODUCT_SIZE];
    char product_lot[DAJILOG_CALIBRATION_LOT_SIZE];
    char channel; /* Physical A, B, C or D. */
    uint32_t revision;
    uint32_t content_hash;
    uint16_t min_decivolt;
    uint16_t max_decivolt;
    uint32_t min_discharge_g_min;
    uint32_t max_discharge_g_min;
    uint16_t valid_point_count;
    bool measured;
    bool approved;
} dajilog_calibration_summary_t;

/** @brief Start an empty, read-only-safe catalog. No sample table is auto-installed. */
esp_err_t dajilog_calibration_catalog_init(void);

/** @brief Register synthetic, unapproved summaries for UI testing only. */
esp_err_t dajilog_calibration_catalog_load_test_samples(void);

/** @brief Load full point rows compiled from the 7B calibration CSV. */
esp_err_t dajilog_calibration_catalog_load_builtin_csv(void);

/** @brief Register one validated, unapproved candidate supplied by a future importer. */
esp_err_t dajilog_calibration_catalog_add_candidate(
    const dajilog_calibration_summary_t *candidate);

/** @brief Copy matching candidates; product/lot may be empty to omit that filter. */
size_t dajilog_calibration_catalog_list(
    char channel, const char *product_code, const char *product_lot,
    dajilog_calibration_summary_t *out, size_t capacity);

/** @brief Look up a stable identity, never a UI row index. */
esp_err_t dajilog_calibration_catalog_find(
    char channel, const char *table_id, uint32_t revision,
    dajilog_calibration_summary_t *out);

/** @brief Copy exact measurement rows for one stable profile identity. */
esp_err_t dajilog_calibration_catalog_get_points(
    char channel, const char *table_id, uint32_t revision,
    dajilog_calibration_point_t *out, size_t capacity, size_t *out_count);

/** @brief Calculate a display-only voltage from the accepted table; never sends CAN. */
esp_err_t dajilog_calibration_catalog_preview_voltage(
    char channel, const char *table_id, uint32_t revision, uint32_t target_g_min,
    uint16_t *out_decivolt, uint32_t *out_expected_g_min);

/** @brief Interpolate estimated fertilizer application rate from a confirmed output voltage. */
esp_err_t dajilog_calibration_catalog_estimate_rate(
    char channel, const char *table_id, uint32_t revision,
    uint16_t measured_decivolt, float *out_g_min);

#ifdef __cplusplus
}
#endif
