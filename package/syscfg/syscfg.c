/**
 * @file syscfg.c
 *
 * @brief The module that reads and writes system configuration from/to flash (syscfg partition).
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "syscfg.h"

int syscfg_init(void) {
  return syscfg_init_impl();
}

int syscfg_open(void) {
  return syscfg_open_impl();
}

int syscfg_close(void) {
  return syscfg_close_impl();
}

int syscfg_set(syscfg_type_t type, const char *key, const char *value) {
  return syscfg_set_impl(type, key, value);
}

int syscfg_get(syscfg_type_t type, const char *key, char *value, size_t value_len) {
  return syscfg_get_impl(type, key, value, value_len);
}

int syscfg_unset(syscfg_type_t type, const char *key) {
  return syscfg_unset_impl(type, key);
}

int syscfg_set_blob(syscfg_type_t type, const char *key, const void *value, size_t value_len) {
  return syscfg_set_blob_impl(type, key, value, value_len);
}

int syscfg_get_blob_size(syscfg_type_t type, const char *key) {
  return syscfg_get_blob_size_impl(type, key);
}

int syscfg_get_blob(syscfg_type_t type, const char *key, void *value, size_t value_len) {
  return syscfg_get_blob_impl(type, key, value, value_len);
}

int syscfg_clear(syscfg_type_t type) {
  return syscfg_clear_impl(type);
}

int syscfg_show(syscfg_type_t type) {
  return syscfg_show_impl(type);
}

int syscfg_info(syscfg_type_t type) {
  return syscfg_info_impl(type);
}

int nvs_erase(void) {
  return nvs_erase_impl();
}
