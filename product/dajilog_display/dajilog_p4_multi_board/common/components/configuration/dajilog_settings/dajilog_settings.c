#include "dajilog_settings.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"

#define SETTINGS_NVS_NAMESPACE "dajilog_cfg"
#define SETTINGS_SCHEMA_KEY "schema_ver"
#define SETTINGS_SCHEMA_VERSION 1U

typedef struct {
    dajilog_setting_value_t current;
    dajilog_setting_value_t draft;
    bool dirty;
} setting_state_t;

static setting_state_t s_states[DAJILOG_SETTING_COUNT];
static SemaphoreHandle_t s_lock;
static bool s_initialized;

static bool value_is_valid(const dajilog_setting_schema_t *schema,
                           const dajilog_setting_value_t *value);

static void make_nvs_key(size_t index, char key[8])
{
    (void)snprintf(key, 8, "s%u", (unsigned int)index);
}

static esp_err_t nvs_read_value(nvs_handle_t handle, size_t index,
                                const dajilog_setting_schema_t *schema,
                                dajilog_setting_value_t *value)
{
    char key[8];
    make_nvs_key(index, key);
    value->type = schema->type;

    switch (schema->type) {
    case DAJILOG_SETTING_INT:
        return nvs_get_i32(handle, key, &value->value.integer);
    case DAJILOG_SETTING_FLOAT: {
        uint32_t bits = 0;
        esp_err_t error = nvs_get_u32(handle, key, &bits);
        if (error == ESP_OK) {
            memcpy(&value->value.real, &bits, sizeof(bits));
        }
        return error;
    }
    case DAJILOG_SETTING_BOOL: {
        uint8_t boolean = 0;
        esp_err_t error = nvs_get_u8(handle, key, &boolean);
        if (error == ESP_OK) {
            value->value.boolean = boolean != 0;
        }
        return error;
    }
    case DAJILOG_SETTING_STRING: {
        size_t length = DAJILOG_SETTING_STRING_CAPACITY;
        return nvs_get_str(handle, key, value->value.string, &length);
    }
    case DAJILOG_SETTING_ENUM:
        return nvs_get_i32(handle, key, &value->value.enumeration);
    default:
        return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t nvs_write_value(nvs_handle_t handle, size_t index,
                                 const dajilog_setting_schema_t *schema,
                                 const dajilog_setting_value_t *value)
{
    char key[8];
    make_nvs_key(index, key);

    switch (schema->type) {
    case DAJILOG_SETTING_INT:
        return nvs_set_i32(handle, key, value->value.integer);
    case DAJILOG_SETTING_FLOAT: {
        uint32_t bits = 0;
        memcpy(&bits, &value->value.real, sizeof(bits));
        return nvs_set_u32(handle, key, bits);
    }
    case DAJILOG_SETTING_BOOL:
        return nvs_set_u8(handle, key, value->value.boolean ? 1U : 0U);
    case DAJILOG_SETTING_STRING:
        return nvs_set_str(handle, key, value->value.string);
    case DAJILOG_SETTING_ENUM:
        return nvs_set_i32(handle, key, value->value.enumeration);
    default:
        return ESP_ERR_INVALID_ARG;
    }
}

static esp_err_t restore_persistent_values(void)
{
    nvs_handle_t handle = 0;
    esp_err_t error = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (error != ESP_OK) {
        return error;
    }

    uint32_t version = 0;
    error = nvs_get_u32(handle, SETTINGS_SCHEMA_KEY, &version);
    if (error == ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(handle);
        return ESP_OK;
    }
    if (error != ESP_OK || version != SETTINGS_SCHEMA_VERSION) {
        nvs_close(handle);
        return error == ESP_OK ? ESP_ERR_INVALID_VERSION : error;
    }

    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(index);
        if (schema == NULL || !schema->persistent) {
            continue;
        }

        dajilog_setting_value_t restored = schema->default_value;
        error = nvs_read_value(handle, index, schema, &restored);
        if (error == ESP_ERR_NVS_NOT_FOUND) {
            continue;
        }
        if (error != ESP_OK || !value_is_valid(schema, &restored)) {
            nvs_close(handle);
            return error == ESP_OK ? ESP_ERR_INVALID_ARG : error;
        }
        s_states[index].current = restored;
        s_states[index].draft = restored;
    }

    nvs_close(handle);
    return ESP_OK;
}

static bool id_is_valid(dajilog_setting_id_t id)
{
    return id >= 0 && id < DAJILOG_SETTING_COUNT;
}

static bool value_is_valid(const dajilog_setting_schema_t *schema,
                           const dajilog_setting_value_t *value)
{
    if (schema == NULL || value == NULL || schema->type != value->type) {
        return false;
    }

    switch (schema->type) {
    case DAJILOG_SETTING_INT:
        return value->value.integer >= schema->min_value.value.integer &&
               value->value.integer <= schema->max_value.value.integer;
    case DAJILOG_SETTING_FLOAT:
        return isfinite(value->value.real) &&
               value->value.real >= schema->min_value.value.real &&
               value->value.real <= schema->max_value.value.real;
    case DAJILOG_SETTING_BOOL:
        return true;
    case DAJILOG_SETTING_STRING:
        return memchr(value->value.string, '\0',
                      DAJILOG_SETTING_STRING_CAPACITY) != NULL;
    case DAJILOG_SETTING_ENUM:
        return value->value.enumeration >= schema->min_value.value.enumeration &&
               value->value.enumeration <= schema->max_value.value.enumeration;
    default:
        return false;
    }
}

static bool value_equals(const dajilog_setting_value_t *left,
                         const dajilog_setting_value_t *right)
{
    if (left->type != right->type) {
        return false;
    }

    switch (left->type) {
    case DAJILOG_SETTING_INT:
        return left->value.integer == right->value.integer;
    case DAJILOG_SETTING_FLOAT:
        return left->value.real == right->value.real;
    case DAJILOG_SETTING_BOOL:
        return left->value.boolean == right->value.boolean;
    case DAJILOG_SETTING_STRING:
        return strncmp(left->value.string, right->value.string,
                       DAJILOG_SETTING_STRING_CAPACITY) == 0;
    case DAJILOG_SETTING_ENUM:
        return left->value.enumeration == right->value.enumeration;
    default:
        return false;
    }
}

esp_err_t dajilog_settings_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
    if (lock == NULL) {
        return ESP_ERR_NO_MEM;
    }

    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(index);
        if (schema == NULL || !value_is_valid(schema, &schema->default_value)) {
            vSemaphoreDelete(lock);
            return ESP_ERR_INVALID_STATE;
        }
        s_states[index].current = schema->default_value;
        s_states[index].draft = schema->default_value;
        s_states[index].dirty = false;
    }

    esp_err_t error = nvs_flash_init();
    if (error != ESP_OK) {
        vSemaphoreDelete(lock);
        return error;
    }

    error = restore_persistent_values();
    if (error == ESP_ERR_INVALID_VERSION) {
        // Keep schema defaults until an explicit migration is implemented.
        error = ESP_OK;
    }
    if (error != ESP_OK) {
        vSemaphoreDelete(lock);
        return error;
    }

    s_lock = lock;
    s_initialized = true;
    return ESP_OK;
}

bool dajilog_settings_is_initialized(void)
{
    return s_initialized;
}

esp_err_t dajilog_settings_get(dajilog_setting_id_t id,
                               dajilog_setting_value_t *value)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!id_is_valid(id) || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    *value = s_states[id].current;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_get_draft(dajilog_setting_id_t id,
                                     dajilog_setting_value_t *value)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!id_is_valid(id) || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    *value = s_states[id].draft;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_set_draft(
    dajilog_setting_id_t id,
    const dajilog_setting_value_t *value)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!id_is_valid(id) || value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(id);
    if (!value_is_valid(schema, value)) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_states[id].draft = *value;
    s_states[id].dirty = !value_equals(&s_states[id].current, value);
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_apply(dajilog_setting_id_t id)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!id_is_valid(id)) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_states[id].current = s_states[id].draft;
    s_states[id].dirty = false;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_apply_all(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(index);
        if (!value_is_valid(schema, &s_states[index].draft)) {
            xSemaphoreGive(s_lock);
            return ESP_ERR_INVALID_ARG;
        }
    }
    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        s_states[index].current = s_states[index].draft;
        s_states[index].dirty = false;
    }
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_save_all(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(index);
        if (!value_is_valid(schema, &s_states[index].draft)) {
            xSemaphoreGive(s_lock);
            return ESP_ERR_INVALID_ARG;
        }
    }

    nvs_handle_t handle = 0;
    esp_err_t error = nvs_open(SETTINGS_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (error == ESP_OK) {
        error = nvs_set_u32(handle, SETTINGS_SCHEMA_KEY,
                            SETTINGS_SCHEMA_VERSION);
    }
    for (size_t index = 0;
         error == ESP_OK && index < DAJILOG_SETTING_COUNT; ++index) {
        const dajilog_setting_schema_t *schema = dajilog_setting_schema_get(index);
        if (schema->persistent) {
            error = nvs_write_value(handle, index, schema,
                                    &s_states[index].draft);
        }
    }
    if (error == ESP_OK) {
        error = nvs_commit(handle);
    }
    if (handle != 0) {
        nvs_close(handle);
    }
    if (error != ESP_OK) {
        xSemaphoreGive(s_lock);
        return error;
    }

    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        s_states[index].current = s_states[index].draft;
        s_states[index].dirty = false;
    }
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_discard_draft(dajilog_setting_id_t id)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!id_is_valid(id)) {
        return ESP_ERR_INVALID_ARG;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_states[id].draft = s_states[id].current;
    s_states[id].dirty = false;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_settings_discard_all_drafts(void)
{
    if (!s_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    for (size_t index = 0; index < DAJILOG_SETTING_COUNT; ++index) {
        s_states[index].draft = s_states[index].current;
        s_states[index].dirty = false;
    }
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

bool dajilog_settings_is_dirty(dajilog_setting_id_t id)
{
    if (!s_initialized || !id_is_valid(id)) {
        return false;
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    const bool dirty = s_states[id].dirty;
    xSemaphoreGive(s_lock);
    return dirty;
}

esp_err_t dajilog_settings_get_float(dajilog_setting_id_t id, float *value)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dajilog_setting_value_t setting;
    esp_err_t error = dajilog_settings_get(id, &setting);
    if (error != ESP_OK) {
        return error;
    }
    if (setting.type != DAJILOG_SETTING_FLOAT) {
        return ESP_ERR_INVALID_ARG;
    }

    *value = setting.value.real;
    return ESP_OK;
}

esp_err_t dajilog_settings_get_int(dajilog_setting_id_t id, int32_t *value)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dajilog_setting_value_t setting;
    esp_err_t error = dajilog_settings_get(id, &setting);
    if (error != ESP_OK) {
        return error;
    }
    if (setting.type != DAJILOG_SETTING_INT) {
        return ESP_ERR_INVALID_ARG;
    }

    *value = setting.value.integer;
    return ESP_OK;
}

esp_err_t dajilog_settings_set_draft_float(dajilog_setting_id_t id,
                                           float value)
{
    const dajilog_setting_value_t setting = {
        .type = DAJILOG_SETTING_FLOAT,
        .value.real = value,
    };
    return dajilog_settings_set_draft(id, &setting);
}

esp_err_t dajilog_settings_set_draft_int(dajilog_setting_id_t id,
                                         int32_t value)
{
    const dajilog_setting_value_t setting = {
        .type = DAJILOG_SETTING_INT,
        .value.integer = value,
    };
    return dajilog_settings_set_draft(id, &setting);
}
