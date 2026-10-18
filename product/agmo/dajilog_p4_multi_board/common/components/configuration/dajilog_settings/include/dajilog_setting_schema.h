#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "dajilog_setting_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    dajilog_setting_id_t id;
    dajilog_setting_category_t category;
    const char *key;
    const char *display_name;
    const char *unit;
    dajilog_setting_type_t type;
    dajilog_setting_value_t default_value;
    dajilog_setting_value_t min_value;
    dajilog_setting_value_t max_value;
    bool persistent;
    bool requires_reboot;
} dajilog_setting_schema_t;

const dajilog_setting_schema_t *dajilog_setting_schema_get(
    dajilog_setting_id_t id);

const dajilog_setting_schema_t *dajilog_setting_schema_find(
    const char *key);

size_t dajilog_setting_schema_count(void);

#ifdef __cplusplus
}
#endif
