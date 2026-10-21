#pragma once

#include "dajilog_table_schema.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*dajilog_table_iterate_cb_t)(dajilog_row_id_t row_id,
                                           const void *row,
                                           size_t row_size,
                                           void *context);

/** Initialize the process-wide table registry. Idempotent. */
esp_err_t dajilog_table_engine_init(void);

/** Register a static schema and allocate its RAM row storage. */
esp_err_t dajilog_table_register(const dajilog_table_schema_t *schema);

const dajilog_table_schema_t *dajilog_table_get_schema(
    dajilog_table_id_t table_id);

esp_err_t dajilog_table_insert(dajilog_table_id_t table_id,
                               const void *row,
                               size_t row_size,
                               dajilog_row_id_t *out_row_id);

esp_err_t dajilog_table_get(dajilog_table_id_t table_id,
                            dajilog_row_id_t row_id,
                            void *out_row,
                            size_t out_size);

esp_err_t dajilog_table_update(dajilog_table_id_t table_id,
                               dajilog_row_id_t row_id,
                               const void *row,
                               size_t row_size);

esp_err_t dajilog_table_delete(dajilog_table_id_t table_id,
                               dajilog_row_id_t row_id);

esp_err_t dajilog_table_count(dajilog_table_id_t table_id,
                              size_t *out_count);

esp_err_t dajilog_table_get_revision(dajilog_table_id_t table_id,
                                     uint32_t *out_revision);

/** Iterates over snapshots. The callback is invoked without the table lock. */
esp_err_t dajilog_table_iterate(dajilog_table_id_t table_id,
                                dajilog_table_iterate_cb_t callback,
                                void *context);

/** Validate a row without inserting or updating it. */
esp_err_t dajilog_table_validate(dajilog_table_id_t table_id,
                                 const void *row,
                                 size_t row_size);

#ifdef __cplusplus
}
#endif
