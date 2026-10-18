#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_UI_MSG_TEXT_CAPACITY 64U
#define DAJILOG_UI_MSG_BYTES_CAPACITY 32U

typedef enum {
    DAJILOG_UI_MSG_COMMAND = 0,
    DAJILOG_UI_MSG_STATE,
    DAJILOG_UI_MSG_EVENT,
    DAJILOG_UI_MSG_RESULT,
} dajilog_ui_msg_kind_t;

typedef enum {
    DAJILOG_UI_VALUE_NONE = 0,
    DAJILOG_UI_VALUE_BOOL,
    DAJILOG_UI_VALUE_I32,
    DAJILOG_UI_VALUE_U32,
    DAJILOG_UI_VALUE_FLOAT,
    DAJILOG_UI_VALUE_TEXT,
    DAJILOG_UI_VALUE_BYTES,
} dajilog_ui_value_type_t;

typedef enum {
    DAJILOG_UI_RESULT_OK = 0,
    DAJILOG_UI_RESULT_REJECTED,
    DAJILOG_UI_RESULT_TIMEOUT,
    DAJILOG_UI_RESULT_OFFLINE,
    DAJILOG_UI_RESULT_INVALID,
    DAJILOG_UI_RESULT_INTERNAL_ERROR,
} dajilog_ui_result_t;

typedef struct {
    uint8_t data[DAJILOG_UI_MSG_BYTES_CAPACITY];
    size_t size;
} dajilog_ui_bytes_t;

typedef struct {
    dajilog_ui_value_type_t type;
    union {
        bool boolean;
        int32_t i32;
        uint32_t u32;
        float real;
        char text[DAJILOG_UI_MSG_TEXT_CAPACITY];
        dajilog_ui_bytes_t bytes;
    } value;
} dajilog_ui_value_t;

/**
 * Project-independent UI message.
 *
 * `topic` is owned by the product project. Common components transport it
 * without interpreting project-specific command or state identifiers.
 */
typedef struct {
    dajilog_ui_msg_kind_t kind;
    uint32_t topic;
    uint32_t sequence;
    int64_t timestamp_us;
    dajilog_ui_result_t result;
    dajilog_ui_value_t payload;
} dajilog_ui_msg_t;

void dajilog_ui_msg_init(dajilog_ui_msg_t *message,
                         dajilog_ui_msg_kind_t kind,
                         uint32_t topic);

uint32_t dajilog_ui_msg_next_sequence(void);

#ifdef __cplusplus
}
#endif
