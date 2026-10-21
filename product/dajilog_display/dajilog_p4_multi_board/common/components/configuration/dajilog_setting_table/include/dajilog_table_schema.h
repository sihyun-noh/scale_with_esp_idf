#pragma once

#include "dajilog_table_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dajilog_field_id_t id;
    const char *key;
    dajilog_field_type_t type;
    size_t offset;
    size_t size;
    bool required;
    bool has_min;
    bool has_max;
    dajilog_field_limit_t min_value;
    dajilog_field_limit_t max_value;
} dajilog_field_schema_t;

typedef esp_err_t (*dajilog_table_validator_t)(const void *row,
                                               size_t row_size,
                                               void *context);

typedef struct {
    dajilog_table_id_t id;
    const char *key;
    uint32_t schema_version;
    size_t row_size;
    size_t max_rows;
    const dajilog_field_schema_t *fields;
    size_t field_count;
    dajilog_table_validator_t validator;
    void *validator_context;
} dajilog_table_schema_t;

#ifdef __cplusplus
}
#endif
