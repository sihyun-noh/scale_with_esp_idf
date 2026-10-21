#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t dajilog_table_id_t;
typedef uint32_t dajilog_field_id_t;
typedef uint32_t dajilog_row_id_t;

#define DAJILOG_TABLE_ID_INVALID ((dajilog_table_id_t)0U)
#define DAJILOG_ROW_ID_INVALID ((dajilog_row_id_t)0U)

typedef enum {
    DAJILOG_FIELD_I32 = 0,
    DAJILOG_FIELD_U32,
    DAJILOG_FIELD_F32,
    DAJILOG_FIELD_BOOL,
    DAJILOG_FIELD_STRING,
    DAJILOG_FIELD_ENUM,
} dajilog_field_type_t;

typedef union {
    int32_t i32;
    uint32_t u32;
    float f32;
    bool boolean;
} dajilog_field_limit_t;

typedef enum {
    DAJILOG_TABLE_EVENT_INSERTED = 0,
    DAJILOG_TABLE_EVENT_UPDATED,
    DAJILOG_TABLE_EVENT_DELETED,
    DAJILOG_TABLE_EVENT_IMPORTED,
} dajilog_table_event_type_t;

typedef struct {
    dajilog_table_id_t table_id;
    dajilog_row_id_t row_id;
    uint32_t revision;
    dajilog_table_event_type_t type;
} dajilog_table_event_t;

#ifdef __cplusplus
}
#endif
