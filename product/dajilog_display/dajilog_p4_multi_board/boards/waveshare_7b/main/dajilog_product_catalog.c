#include "dajilog_product_catalog.h"

#include <string.h>

static const dajilog_product_entry_t s_products[] = {
#include "dajilog_product_catalog_data.inc"
};

/** @brief Copy catalog entries so UI code never reads build data directly. */
esp_err_t dajilog_product_catalog_get(dajilog_product_catalog_t *catalog)
{
    if (catalog == NULL) return ESP_ERR_INVALID_ARG;
    _Static_assert(sizeof(s_products) / sizeof(s_products[0]) <= DAJILOG_PRODUCT_CATALOG_LIMIT,
                   "product catalog exceeds snapshot capacity");
    memset(catalog, 0, sizeof(*catalog));
    for (size_t i = 0; i < sizeof(s_products) / sizeof(s_products[0]); ++i) {
        if (s_products[i].enabled)
            catalog->products[catalog->count++] = s_products[i];
    }
    return ESP_OK;
}

/** @brief Resolve a persistent code to its current enabled catalog entry. */
const dajilog_product_entry_t *dajilog_product_catalog_find(const char *code)
{
    if (code == NULL || code[0] == '\0') return NULL;
    for (size_t i = 0; i < sizeof(s_products) / sizeof(s_products[0]); ++i) {
        if (s_products[i].enabled && strcmp(s_products[i].code, code) == 0)
            return &s_products[i];
    }
    return NULL;
}
