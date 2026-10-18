#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DAJILOG_SETTING_STRING_CAPACITY 64U

typedef enum {
    DAJILOG_SETTING_INT = 0,
    DAJILOG_SETTING_FLOAT,
    DAJILOG_SETTING_BOOL,
    DAJILOG_SETTING_STRING,
    DAJILOG_SETTING_ENUM,
} dajilog_setting_type_t;

typedef enum {
    DAJILOG_SETTING_CATEGORY_SYSTEM = 0,
    DAJILOG_SETTING_CATEGORY_OPERATION,
    DAJILOG_SETTING_CATEGORY_COMMUNICATION,
    DAJILOG_SETTING_CATEGORY_DISPLAY,
    DAJILOG_SETTING_CATEGORY_CALIBRATION,
    DAJILOG_SETTING_CATEGORY_SERVICE,
} dajilog_setting_category_t;

typedef enum {
    DAJILOG_SETTING_MANUAL_TARGET = 0,
    DAJILOG_SETTING_RECIPE_RIDGE_COUNT,
    DAJILOG_SETTING_RECIPE_RIDGE_WIDTH_M,
    DAJILOG_SETTING_RECIPE_ACTUAL_LENGTH_M,
    DAJILOG_SETTING_RECIPE_NITROGEN_RATE_KG_N_PER_10A,
    DAJILOG_SETTING_COUNT,
} dajilog_setting_id_t;

typedef struct {
    dajilog_setting_type_t type;
    union {
        int32_t integer;
        float real;
        bool boolean;
        int32_t enumeration;
        char string[DAJILOG_SETTING_STRING_CAPACITY];
    } value;
} dajilog_setting_value_t;

#ifdef __cplusplus
}
#endif
