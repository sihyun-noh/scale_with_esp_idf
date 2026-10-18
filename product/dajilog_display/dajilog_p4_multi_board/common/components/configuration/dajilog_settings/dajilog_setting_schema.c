#include "dajilog_setting_schema.h"

#include <string.h>

#define FLOAT_VALUE(number) \
    { .type = DAJILOG_SETTING_FLOAT, .value.real = (number) }
#define INT_VALUE(number) \
    { .type = DAJILOG_SETTING_INT, .value.integer = (number) }

static const dajilog_setting_schema_t s_schema[DAJILOG_SETTING_COUNT] = {
    [DAJILOG_SETTING_MANUAL_TARGET] = {
        .id = DAJILOG_SETTING_MANUAL_TARGET,
        .category = DAJILOG_SETTING_CATEGORY_OPERATION,
        .key = "manual_target",
        .display_name = "수동 목표값",
        .unit = "kg",
        .type = DAJILOG_SETTING_FLOAT,
        .default_value = FLOAT_VALUE(10.0f),
        .min_value = FLOAT_VALUE(0.0f),
        .max_value = FLOAT_VALUE(1000.0f),
        .persistent = true,
        .requires_reboot = false,
    },
    [DAJILOG_SETTING_RECIPE_RIDGE_COUNT] = {
        .id = DAJILOG_SETTING_RECIPE_RIDGE_COUNT,
        .category = DAJILOG_SETTING_CATEGORY_OPERATION,
        .key = "recipe_ridge_count",
        .display_name = "두둑 수",
        .unit = "개",
        .type = DAJILOG_SETTING_INT,
        .default_value = INT_VALUE(6),
        .min_value = INT_VALUE(1),
        .max_value = INT_VALUE(999),
        .persistent = true,
        .requires_reboot = false,
    },
    [DAJILOG_SETTING_RECIPE_RIDGE_WIDTH_M] = {
        .id = DAJILOG_SETTING_RECIPE_RIDGE_WIDTH_M,
        .category = DAJILOG_SETTING_CATEGORY_OPERATION,
        .key = "recipe_ridge_width_m",
        .display_name = "두둑 폭",
        .unit = "m",
        .type = DAJILOG_SETTING_FLOAT,
        .default_value = FLOAT_VALUE(1.20f),
        .min_value = FLOAT_VALUE(0.1f),
        .max_value = FLOAT_VALUE(100.0f),
        .persistent = true,
        .requires_reboot = false,
    },
    [DAJILOG_SETTING_RECIPE_ACTUAL_LENGTH_M] = {
        .id = DAJILOG_SETTING_RECIPE_ACTUAL_LENGTH_M,
        .category = DAJILOG_SETTING_CATEGORY_OPERATION,
        .key = "recipe_actual_length_m",
        .display_name = "실제 길이",
        .unit = "m",
        .type = DAJILOG_SETTING_FLOAT,
        .default_value = FLOAT_VALUE(40.0f),
        .min_value = FLOAT_VALUE(0.1f),
        .max_value = FLOAT_VALUE(100000.0f),
        .persistent = true,
        .requires_reboot = false,
    },
    [DAJILOG_SETTING_RECIPE_NITROGEN_RATE_KG_N_PER_10A] = {
        .id = DAJILOG_SETTING_RECIPE_NITROGEN_RATE_KG_N_PER_10A,
        .category = DAJILOG_SETTING_CATEGORY_OPERATION,
        .key = "recipe_nitrogen_rate_kg_n_per_10a",
        .display_name = "질소 시비량",
        .unit = "kg N/10a",
        .type = DAJILOG_SETTING_FLOAT,
        .default_value = FLOAT_VALUE(0.0f),
        .min_value = FLOAT_VALUE(0.0f),
        .max_value = FLOAT_VALUE(1000.0f),
        .persistent = true,
        .requires_reboot = false,
    },
};

const dajilog_setting_schema_t *dajilog_setting_schema_get(
    dajilog_setting_id_t id)
{
    if (id < 0 || id >= DAJILOG_SETTING_COUNT) {
        return NULL;
    }
    return &s_schema[id];
}

const dajilog_setting_schema_t *dajilog_setting_schema_find(const char *key)
{
    if (key == NULL) {
        return NULL;
    }

    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        if (strcmp(s_schema[index].key, key) == 0) {
            return &s_schema[index];
        }
    }
    return NULL;
}

size_t dajilog_setting_schema_count(void)
{
    return DAJILOG_SETTING_COUNT;
}
