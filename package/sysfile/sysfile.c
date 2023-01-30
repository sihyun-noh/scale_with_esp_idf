/**
 * @file sysfile.c
 *
 * @brief using SPIFFS is provided in the "storage/spiffs" directory.
 * This is initializes and mounts/format a SPIFFS partition, then writes and reads data from it using POSIX and C
 library APIs.
 *
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include "sysfile.h"
#if defined(CONFIG_SPIFFS_PACKAGE)
#include "esp32/spiffs_impl.h"
#elif defined(CONFIG_LITTLEFS_PACKAGE)
#include "esp32/littlefs_impl.h"
#endif

int init_sysfile(const char *partition_name, const char *root_path) {
  int ret = 0;

#if defined(CONFIG_SPIFFS_PACKAGE)
  ret = init_spiffs_impl(partition_name, root_path);
#elif defined(CONFIG_LITTLEFS_PACKAGE)
  ret = init_littlefs_impl(partition_name, root_path);
#endif
  return ret;
}

int sysfile_format(void) {
  int ret = 0;

#if defined(CONFIG_SPIFFS_PACKAGE)
  ret = format_spiffs_impl();
#elif defined(CONFIG_LITTLEFS_PACKAGE)
  ret = format_littlefs_impl();
#endif
  return ret;
}

int sysfile_show_file(void) {
  return show_file_impl();
}

int write_log(const char *log_file_name, const char *log_data) {
  return write_log_data_to_file_impl(log_file_name, log_data);
}
