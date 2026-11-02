#pragma once

#include <stddef.h>

#include "dajilog_product_types.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Return the build-time sample catalog generated from the product CSV. */
esp_err_t dajilog_product_catalog_get(dajilog_product_catalog_t *catalog);

/** @brief Find one enabled product by stable code. */
const dajilog_product_entry_t *dajilog_product_catalog_find(const char *code);

#ifdef __cplusplus
}
#endif
