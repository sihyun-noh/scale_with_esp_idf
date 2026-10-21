#include "dajilog_table.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define DAJILOG_TABLE_REGISTRY_CAPACITY 16U

typedef struct {
    bool occupied;
    dajilog_row_id_t row_id;
} row_slot_t;

typedef struct {
    const dajilog_table_schema_t *schema;
    uint8_t *rows;
    row_slot_t *slots;
    size_t count;
    dajilog_row_id_t next_row_id;
    uint32_t revision;
} table_state_t;

static table_state_t s_tables[DAJILOG_TABLE_REGISTRY_CAPACITY];
static SemaphoreHandle_t s_lock;

static table_state_t *find_table(dajilog_table_id_t table_id)
{
    for (size_t i = 0; i < DAJILOG_TABLE_REGISTRY_CAPACITY; ++i) {
        if (s_tables[i].schema != NULL && s_tables[i].schema->id == table_id) {
            return &s_tables[i];
        }
    }
    return NULL;
}

static bool field_bounds_valid(const dajilog_field_schema_t *field,
                               size_t row_size)
{
    return field != NULL && field->key != NULL && field->size > 0 &&
           field->offset <= row_size && field->size <= row_size - field->offset;
}

static bool scalar_valid(const dajilog_field_schema_t *field, const void *value)
{
    switch (field->type) {
    case DAJILOG_FIELD_I32: {
        if (field->size != sizeof(int32_t)) return false;
        const int32_t v = *(const int32_t *)value;
        return (!field->has_min || v >= field->min_value.i32) &&
               (!field->has_max || v <= field->max_value.i32);
    }
    case DAJILOG_FIELD_U32: {
        if (field->size != sizeof(uint32_t)) return false;
        const uint32_t v = *(const uint32_t *)value;
        return (!field->has_min || v >= field->min_value.u32) &&
               (!field->has_max || v <= field->max_value.u32);
    }
    case DAJILOG_FIELD_F32: {
        if (field->size != sizeof(float)) return false;
        const float v = *(const float *)value;
        return isfinite(v) && (!field->has_min || v >= field->min_value.f32) &&
               (!field->has_max || v <= field->max_value.f32);
    }
    case DAJILOG_FIELD_BOOL:
        return field->size == sizeof(bool);
    case DAJILOG_FIELD_ENUM: {
        if (field->size != sizeof(int32_t)) return false;
        const int32_t v = *(const int32_t *)value;
        return (!field->has_min || v >= field->min_value.i32) &&
               (!field->has_max || v <= field->max_value.i32);
    }
    case DAJILOG_FIELD_STRING: {
        const char *text = (const char *)value;
        const char *end = memchr(text, '\0', field->size);
        return end != NULL && (!field->required || text[0] != '\0');
    }
    default:
        return false;
    }
}

static esp_err_t validate_row(const dajilog_table_schema_t *schema,
                              const void *row,
                              size_t row_size)
{
    if (schema == NULL || row == NULL || row_size != schema->row_size) {
        return ESP_ERR_INVALID_ARG;
    }
    const uint8_t *bytes = (const uint8_t *)row;
    for (size_t i = 0; i < schema->field_count; ++i) {
        const dajilog_field_schema_t *field = &schema->fields[i];
        if (!field_bounds_valid(field, row_size) ||
            !scalar_valid(field, bytes + field->offset)) {
            return ESP_ERR_INVALID_ARG;
        }
    }
    return schema->validator != NULL
               ? schema->validator(row, row_size, schema->validator_context)
               : ESP_OK;
}

static void bump_revision(table_state_t *table)
{
    if (++table->revision == 0) table->revision = 1;
}

esp_err_t dajilog_table_engine_init(void)
{
    if (s_lock != NULL) return ESP_OK;
    SemaphoreHandle_t lock = xSemaphoreCreateMutex();
    if (lock == NULL) return ESP_ERR_NO_MEM;
    s_lock = lock;
    return ESP_OK;
}

esp_err_t dajilog_table_register(const dajilog_table_schema_t *schema)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (schema == NULL || schema->id == DAJILOG_TABLE_ID_INVALID ||
        schema->key == NULL || schema->key[0] == '\0' || schema->row_size == 0 ||
        schema->max_rows == 0 || schema->fields == NULL || schema->field_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    for (size_t i = 0; i < schema->field_count; ++i) {
        if (!field_bounds_valid(&schema->fields[i], schema->row_size)) {
            return ESP_ERR_INVALID_ARG;
        }
    }

    xSemaphoreTake(s_lock, portMAX_DELAY);
    if (find_table(schema->id) != NULL) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_INVALID_STATE;
    }
    table_state_t *target = NULL;
    for (size_t i = 0; i < DAJILOG_TABLE_REGISTRY_CAPACITY; ++i) {
        if (s_tables[i].schema == NULL) {
            target = &s_tables[i];
            break;
        }
    }
    if (target == NULL || schema->max_rows > SIZE_MAX / schema->row_size) {
        xSemaphoreGive(s_lock);
        return target == NULL ? ESP_ERR_NO_MEM : ESP_ERR_INVALID_SIZE;
    }
    uint8_t *rows = calloc(schema->max_rows, schema->row_size);
    row_slot_t *slots = calloc(schema->max_rows, sizeof(*slots));
    if (rows == NULL || slots == NULL) {
        free(rows);
        free(slots);
        xSemaphoreGive(s_lock);
        return ESP_ERR_NO_MEM;
    }
    *target = (table_state_t){
        .schema = schema,
        .rows = rows,
        .slots = slots,
        .next_row_id = 1,
        .revision = 1,
    };
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

const dajilog_table_schema_t *dajilog_table_get_schema(dajilog_table_id_t table_id)
{
    if (s_lock == NULL) return NULL;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    const dajilog_table_schema_t *schema = table != NULL ? table->schema : NULL;
    xSemaphoreGive(s_lock);
    return schema;
}

esp_err_t dajilog_table_validate(dajilog_table_id_t table_id,
                                 const void *row,
                                 size_t row_size)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    esp_err_t error = table != NULL ? validate_row(table->schema, row, row_size)
                                    : ESP_ERR_NOT_FOUND;
    xSemaphoreGive(s_lock);
    return error;
}

esp_err_t dajilog_table_insert(dajilog_table_id_t table_id, const void *row,
                               size_t row_size, dajilog_row_id_t *out_row_id)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (row == NULL || out_row_id == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table == NULL) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t error = validate_row(table->schema, row, row_size);
    if (error != ESP_OK || table->count == table->schema->max_rows) {
        xSemaphoreGive(s_lock);
        return error != ESP_OK ? error : ESP_ERR_NO_MEM;
    }
    size_t slot = 0;
    while (table->slots[slot].occupied) ++slot;
    dajilog_row_id_t id = table->next_row_id++;
    if (id == DAJILOG_ROW_ID_INVALID) id = table->next_row_id++;
    memcpy(table->rows + slot * table->schema->row_size, row, row_size);
    table->slots[slot] = (row_slot_t){.occupied = true, .row_id = id};
    ++table->count;
    bump_revision(table);
    *out_row_id = id;
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

static size_t find_slot(const table_state_t *table, dajilog_row_id_t row_id)
{
    for (size_t i = 0; i < table->schema->max_rows; ++i) {
        if (table->slots[i].occupied && table->slots[i].row_id == row_id) return i;
    }
    return SIZE_MAX;
}

esp_err_t dajilog_table_get(dajilog_table_id_t table_id, dajilog_row_id_t row_id,
                            void *out_row, size_t out_size)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (row_id == DAJILOG_ROW_ID_INVALID || out_row == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table == NULL || out_size != table->schema->row_size) {
        xSemaphoreGive(s_lock);
        return table == NULL ? ESP_ERR_NOT_FOUND : ESP_ERR_INVALID_SIZE;
    }
    const size_t slot = find_slot(table, row_id);
    if (slot == SIZE_MAX) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    memcpy(out_row, table->rows + slot * table->schema->row_size, out_size);
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_table_update(dajilog_table_id_t table_id, dajilog_row_id_t row_id,
                               const void *row, size_t row_size)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (row_id == DAJILOG_ROW_ID_INVALID || row == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table == NULL) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    esp_err_t error = validate_row(table->schema, row, row_size);
    const size_t slot = find_slot(table, row_id);
    if (error != ESP_OK || slot == SIZE_MAX) {
        xSemaphoreGive(s_lock);
        return error != ESP_OK ? error : ESP_ERR_NOT_FOUND;
    }
    memcpy(table->rows + slot * table->schema->row_size, row, row_size);
    bump_revision(table);
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_table_delete(dajilog_table_id_t table_id, dajilog_row_id_t row_id)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (row_id == DAJILOG_ROW_ID_INVALID) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    const size_t slot = table != NULL ? find_slot(table, row_id) : SIZE_MAX;
    if (table == NULL || slot == SIZE_MAX) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    memset(table->rows + slot * table->schema->row_size, 0, table->schema->row_size);
    table->slots[slot] = (row_slot_t){0};
    --table->count;
    bump_revision(table);
    xSemaphoreGive(s_lock);
    return ESP_OK;
}

esp_err_t dajilog_table_count(dajilog_table_id_t table_id, size_t *out_count)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (out_count == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table != NULL) *out_count = table->count;
    xSemaphoreGive(s_lock);
    return table != NULL ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t dajilog_table_get_revision(dajilog_table_id_t table_id,
                                     uint32_t *out_revision)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (out_revision == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table != NULL) *out_revision = table->revision;
    xSemaphoreGive(s_lock);
    return table != NULL ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t dajilog_table_iterate(dajilog_table_id_t table_id,
                                dajilog_table_iterate_cb_t callback,
                                void *context)
{
    if (s_lock == NULL) return ESP_ERR_INVALID_STATE;
    if (callback == NULL) return ESP_ERR_INVALID_ARG;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    table_state_t *table = find_table(table_id);
    if (table == NULL) {
        xSemaphoreGive(s_lock);
        return ESP_ERR_NOT_FOUND;
    }
    const size_t row_size = table->schema->row_size;
    const size_t max_rows = table->schema->max_rows;
    uint8_t *rows = malloc(max_rows * row_size);
    row_slot_t *slots = malloc(max_rows * sizeof(*slots));
    if (rows == NULL || slots == NULL) {
        free(rows);
        free(slots);
        xSemaphoreGive(s_lock);
        return ESP_ERR_NO_MEM;
    }
    memcpy(rows, table->rows, max_rows * row_size);
    memcpy(slots, table->slots, max_rows * sizeof(*slots));
    xSemaphoreGive(s_lock);

    for (size_t i = 0; i < max_rows; ++i) {
        if (slots[i].occupied &&
            !callback(slots[i].row_id, rows + i * row_size, row_size, context)) {
            break;
        }
    }
    free(rows);
    free(slots);
    return ESP_OK;
}
