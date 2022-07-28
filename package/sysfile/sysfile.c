/**
 * @file sysfile.c
 *
 * @brief using SPIFFS is provided in the "storage/spiffs" directory. 
 * This is initializes and mounts/format a SPIFFS partition, then writes and reads data from it using POSIX and C library APIs. 
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

int init_sysfile(void) {
  return init_spiffs_impl();
}

int sysfile_format(void) {
  return format_spiffs_impl();
}

int write_log(const char *log_file_name, const char *log_data) {
  return write_log_data_to_file_impl(log_file_name, log_data);
}
