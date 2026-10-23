#pragma once

#include "dajilog_table_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*dajilog_table_iterate_cb_t)(dajilog_row_id_t row_id,
                                           const void *row,
                                           size_t row_size,
                                           void *context);

/** @brief Initialize the process-wide table registry. Idempotent. */
esp_err_t dajilog_table_engine_init(void);

/** @brief Register a static schema and allocate its RAM row storage. */
esp_err_t dajilog_table_register(const dajilog_table_schema_t *schema);

/** @brief Return the immutable schema registered for a table ID. */
const dajilog_table_schema_t *dajilog_table_get_schema(
    dajilog_table_id_t table_id);

/** @brief Validate and insert a row, assigning a stable runtime row ID. */
esp_err_t dajilog_table_insert(dajilog_table_id_t table_id,
                               const void *row,
                               size_t row_size,
                               dajilog_row_id_t *out_row_id);

/** @brief Copy one row into a caller-owned output buffer. */
esp_err_t dajilog_table_get(dajilog_table_id_t table_id,
                            dajilog_row_id_t row_id,
                            void *out_row,
                            size_t out_size);

/** @brief Validate and replace an existing row. */
esp_err_t dajilog_table_update(dajilog_table_id_t table_id,
                               dajilog_row_id_t row_id,
                               const void *row,
                               size_t row_size);

/** @brief Delete an existing row and advance the table revision. */
esp_err_t dajilog_table_delete(dajilog_table_id_t table_id,
                               dajilog_row_id_t row_id);

/** @brief Return the number of occupied rows in a table. */
esp_err_t dajilog_table_count(dajilog_table_id_t table_id,
                              size_t *out_count);

/** @brief Return the table revision used to detect changes. */
esp_err_t dajilog_table_get_revision(dajilog_table_id_t table_id,
                                     uint32_t *out_revision);

/** @brief Iterate over row snapshots without holding the table lock in callbacks. */
esp_err_t dajilog_table_iterate(dajilog_table_id_t table_id,
                                dajilog_table_iterate_cb_t callback,
                                void *context);

/** @brief Validate a row without inserting or updating it. */
esp_err_t dajilog_table_validate(dajilog_table_id_t table_id,
                                 const void *row,
                                 size_t row_size);

#ifdef __cplusplus
}
#endif
