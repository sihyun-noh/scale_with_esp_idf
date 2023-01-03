/**
 * @file syscfg_impl.c
 *
 * @brief system configuration implementation for esp32 platform
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <string.h>

#include "syscfg_impl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/semphr.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_log.h"

#define CFG_PARTITION_NAME "cfg" /*<! NVS partition name of system configuration */
#define MFG_PARTITION_NAME "mfg" /*<! NVS partition name of manufacturing */

static const char *TAG = "syscfg";

SemaphoreHandle_t syscfg_mutex;

struct syscfg_context {
  char *partition_name;
  char *nvs_name;
  nvs_handle_t handle;

  int (*init)(void *data);
  int (*open)(void *data);
  int (*close)(void *data);
};

static int _syscfg_init(void *data);
static int _syscfg_open(void *data);
static int _syscfg_close(void *data);

static struct syscfg_context syscfg_ctx[] = {
  [CFG_DATA] = {
    .partition_name = CFG_PARTITION_NAME,
    .nvs_name = "cfg",
    .handle = 0,
    .init = _syscfg_init,
    .open = _syscfg_open,
    .close = _syscfg_close,
  },
  [MFG_DATA] = {
    .partition_name = MFG_PARTITION_NAME,
    .nvs_name = "mfg",
    .handle = 0,
    .init = _syscfg_init,
    .open = _syscfg_open,
    .close = _syscfg_close,
  },
};

typedef struct {
  nvs_type_t type;
  const char *str;
} type_str_pair_t;

static const type_str_pair_t type_str_pair[] = {
  { NVS_TYPE_I8, "i8" },   { NVS_TYPE_U8, "u8" },     { NVS_TYPE_U16, "u16" }, { NVS_TYPE_I16, "i16" },
  { NVS_TYPE_U32, "u32" }, { NVS_TYPE_I32, "i32" },   { NVS_TYPE_U64, "u64" }, { NVS_TYPE_I64, "i64" },
  { NVS_TYPE_STR, "str" }, { NVS_TYPE_BLOB, "blob" }, { NVS_TYPE_ANY, "any" },
};

static const size_t TYPE_STR_PAIR_SIZE = sizeof(type_str_pair) / sizeof(type_str_pair[0]);

#if 0
static nvs_type_t str_to_type(const char *type) {
  for (int i = 0; i < TYPE_STR_PAIR_SIZE; i++) {
    const type_str_pair_t *p = &type_str_pair[i];
    if (strcmp(type, p->str) == 0) {
      return p->type;
    }
  }

  return NVS_TYPE_ANY;
}
#endif

static const char *type_to_str(nvs_type_t type) {
  for (int i = 0; i < TYPE_STR_PAIR_SIZE; i++) {
    const type_str_pair_t *p = &type_str_pair[i];
    if (p->type == type) {
      return p->str;
    }
  }

  return "Unknown";
}

static void _get_nvs_partition_name(syscfg_type_t type, char **pp_partition_name, char **pp_nvs_name) {
  if (type >= NO_DATA) {
    ESP_LOGE(TAG, "Invalid data type %d", type);
    return;
  }

  struct syscfg_context *ctx = &syscfg_ctx[type];
  if (ctx && ctx->partition_name && ctx->nvs_name) {
    *pp_partition_name = ctx->partition_name;
    *pp_nvs_name = ctx->nvs_name;
  }
}

static int _get_nvs_handle(syscfg_type_t type) {
  if (type >= NO_DATA) {
    ESP_LOGE(TAG, "Invalid data type %d", type);
    return -1;
  }

  struct syscfg_context *ctx = &syscfg_ctx[type];
  if (ctx && ctx->handle) {
    return ctx->handle;
  }

  esp_err_t ret = nvs_open_from_partition(ctx->partition_name, ctx->nvs_name, NVS_READWRITE, &ctx->handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS handle for %s", ctx->nvs_name);
    return -1;
  }
  return ctx->handle;
}

static int _syscfg_init(void *data) {
  struct syscfg_context *ctx = (struct syscfg_context *)data;
  esp_err_t ret = nvs_flash_init_partition(ctx->partition_name);
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "Failed to init %s NVS partition [%d], Erasing the partition.", ctx->partition_name, ret);
    nvs_flash_erase_partition(ctx->partition_name);
    ret = nvs_flash_init_partition(ctx->partition_name);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to init %s NVS partition [%d]", ctx->partition_name, ret);
      return -1;
    }
  }
  return 0;
}

static int _syscfg_open(void *data) {
  struct syscfg_context *ctx = (struct syscfg_context *)data;
  if (ctx->handle) {
    ESP_LOGI(TAG, "%s handle already open", ctx->nvs_name);
    return 0;
  }

  esp_err_t ret = nvs_open_from_partition(ctx->partition_name, ctx->nvs_name, NVS_READWRITE, &ctx->handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open %s NVS partition [%d]", ctx->nvs_name, ret);
    return -1;
  }
  return 0;
}

static int _syscfg_close(void *data) {
  struct syscfg_context *ctx = (struct syscfg_context *)data;
  if (!ctx->handle) {
    ESP_LOGI(TAG, "%s handle already closed", ctx->nvs_name);
    return 0;
  }
  nvs_close(ctx->handle);
  ctx->handle = 0;
  return 0;
}

int syscfg_init_impl(void) {
  for (int i = 0; i < ARRAY_SIZE(syscfg_ctx); i++) {
    struct syscfg_context *ctx = &syscfg_ctx[i];
    if (ctx->init && ctx->init(ctx) < 0) {
      return SYSCFG_ERR_INIT;
    }
  }
  return SYSCFG_NO_ERR;
}

int syscfg_open_impl(void) {
  if (syscfg_mutex) {
    ESP_LOGE(TAG, "syscfg is already open");
    return SYSCFG_NO_ERR;
  }

  for (int i = 0; i < ARRAY_SIZE(syscfg_ctx); i++) {
    struct syscfg_context *ctx = &syscfg_ctx[i];
    if (ctx->open && ctx->open(ctx) < 0) {
      return SYSCFG_ERR_OPEN;
    }
  }

  syscfg_mutex = xSemaphoreCreateMutex();
  return SYSCFG_NO_ERR;
}

int syscfg_close_impl(void) {
  if (syscfg_mutex == NULL) {
    ESP_LOGE(TAG, "syscfg is already close");
    return SYSCFG_NO_ERR;
  }

  vSemaphoreDelete(syscfg_mutex);
  syscfg_mutex = NULL;

  for (int i = 0; i < ARRAY_SIZE(syscfg_ctx); i++) {
    struct syscfg_context *ctx = &syscfg_ctx[i];
    if (ctx->close && ctx->close(ctx) < 0) {
      return SYSCFG_ERR_CLOSE;
    }
  }

  return SYSCFG_NO_ERR;
}

int syscfg_set_impl(syscfg_type_t type, const char *key, const char *value) {
  int ret = SYSCFG_NO_ERR;

  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_set_str(handle, key, value) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set %s [%d]", key, ret);
    ret = SYSCFG_ERR_SET;
    goto _exit;
  }

  if (nvs_commit(handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit syscfg [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_COMMIT;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_get_impl(syscfg_type_t type, const char *key, char *value, size_t value_len) {
  int ret = SYSCFG_NO_ERR;
  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_get_str(handle, key, value, &value_len) != ESP_OK) {
    value[0] = 0x00;
    ESP_LOGE(TAG, "Failed to get %s [%d]", key, ret);
    ret = SYSCFG_ERR_GET;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_unset_impl(syscfg_type_t type, const char *key) {
  int ret = SYSCFG_NO_ERR;
  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  ret = nvs_erase_key(handle, key);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to unset %s [%d]", key, ret);
    ret = SYSCFG_ERR_ERASE;
    goto _exit;
  }

  ret = nvs_commit(handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit syscfg [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_COMMIT;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_clear_impl(syscfg_type_t type) {
  int ret = SYSCFG_NO_ERR;

  if (type == MFG_DATA) {
    ESP_LOGI(TAG, "Manufacturing data cannot be cleared");
    return ret;
  }

  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_erase_all(handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to clear syscfg [%d]", ret);
    ret = SYSCFG_ERR_ERASE;
    goto _exit;
  }

  if (nvs_commit(handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit syscfg [%d]", ret);
    ret = SYSCFG_ERR_COMMIT;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_set_blob_impl(syscfg_type_t type, const char *key, const void *value, size_t value_len) {
  int ret = SYSCFG_NO_ERR;

  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_set_blob(handle, key, value, value_len) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set blob [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_SET;
    goto _exit;
  }

  if (nvs_commit(handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit syscfg [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_COMMIT;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_get_blob_size_impl(syscfg_type_t type, const char *key) {
  int ret = SYSCFG_NO_ERR;
  size_t value_len = 0;
  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_get_blob(handle, key, NULL, &value_len) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get blob size [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_GET;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return (ret == SYSCFG_NO_ERR) ? value_len : ret;
}

int syscfg_get_blob_impl(syscfg_type_t type, const char *key, void *value, size_t value_len) {
  int ret = SYSCFG_NO_ERR;
  xSemaphoreTake(syscfg_mutex, portMAX_DELAY);

  int handle = _get_nvs_handle(type);
  if (handle < 0) {
    ret = SYSCFG_ERR_NVS_HANDLE;
    goto _exit;
  }

  if (nvs_get_blob(handle, key, value, &value_len) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get blob [%s] [%d]", key, ret);
    ret = SYSCFG_ERR_GET;
  }

_exit:
  xSemaphoreGive(syscfg_mutex);
  return ret;
}

int syscfg_show_impl(syscfg_type_t type) {
  char *partition_name = NULL;
  char *nvs_name = NULL;
#if 0
  char value[256] = { 0 };
#endif

  _get_nvs_partition_name(type, &partition_name, &nvs_name);
  if (NULL == partition_name || NULL == nvs_name) {
    ESP_LOGE(TAG, "Not found partition or nvs name for type %d", type);
    return -1;
  }

#if 0
  int handle = _get_nvs_handle(type);
#endif

  nvs_iterator_t it = NULL;

  esp_err_t result = nvs_entry_find(partition_name, nvs_name, NVS_TYPE_ANY, &it);
  if (result == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGE(TAG, "No such entry was found");
    return -1;
  }

  if (result != ESP_OK) {
    ESP_LOGE(TAG, "NVS error: %s", esp_err_to_name(result));
    return 1;
  }

#if 0
  while (it != NULL) {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    it = nvs_entry_next(it);
    if (info.type == NVS_TYPE_STR) {
      size_t len = sizeof(value);
      nvs_get_str(handle, info.key, value, &len);
      value[len] = 0;
      ESP_LOGI(TAG, "key '%s', value '%s'", info.key, value);
    }
  };
#endif

  do {
    nvs_entry_info_t info;
    nvs_entry_info(it, &info);
    result = nvs_entry_next(&it);

    printf("namespace '%s', key '%s', type '%s' \n", info.namespace_name, info.key, type_to_str(info.type));
  } while (result == ESP_OK);

  if (result != ESP_ERR_NVS_NOT_FOUND) {  // the last iteration ran into an internal error
    ESP_LOGE(TAG, "NVS error %s at current iteration, stopping.", esp_err_to_name(result));
    return -1;
  }
  // Note : No need to release iterator obtained from nvs_entry_find() function
  // when nvs_entry_find or nvs_entry_next functions returns NULL, indicating no
  // other element for specified critieria was found.

  return 0;
}

int syscfg_info_impl(syscfg_type_t type) {
  char *partition_name = NULL;
  char *nvs_name = NULL;

  _get_nvs_partition_name(type, &partition_name, &nvs_name);
  if (NULL == partition_name) {
    ESP_LOGE(TAG, "Not found partition name for type %d", type);
    return -1;
  }

  nvs_stats_t stats;
  esp_err_t ret = nvs_get_stats(partition_name, &stats);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get stats [%d]", ret);
    return -1;
  }

  ESP_LOGI(TAG, "NVS partition %s: used %d, free %d, total %d", partition_name, stats.used_entries, stats.free_entries,
           stats.total_entries);
  return 0;
}

int nvs_erase_impl(void) {
  ESP_LOGI(TAG, "Erasing NVS...\n");

  nvs_handle_t nvs_handle;

  esp_err_t err = nvs_open("nvs", NVS_READWRITE, &nvs_handle);
  if (err == ESP_OK) {
    err = nvs_erase_all(nvs_handle);
    if (err == ESP_OK) {
      err = nvs_commit(nvs_handle);
    }

    nvs_close(nvs_handle);
  }

  ESP_LOGI(TAG, "Namespace '%s' was %s erased\n", "nvs", (err == ESP_OK) ? "" : "not");

  return (err == ESP_OK) ? 0 : -1;
}
