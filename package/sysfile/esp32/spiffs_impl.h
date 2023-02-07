/**
 * @file spiffs_impl.h
 *
 * @brief using SPIFFS is provided in the "storage/spiffs" directory.
 * This is initializes and mounts a SPIFFS partition, then writes and reads data from it using POSIX and C library APIs.
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

#ifndef _SPIFFS_IMPL_H_
#define _SPIFFS_IMPL_H_

#define BASE_PATH "/spiffs"
#define PARTITION_NAME ""

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register and mount SPIFFS to VFS with given path prefix.
 *
 * @return int 0 on success, -1 on failure,
 */

int init_spiffs_impl(const char *partition_name, const char *root_path);

/**
 * @brief Format the SPIFFS partition
 *
 * @return int 0 on success, -1 on failure,
 */
int format_spiffs_impl(void);

/**
 * @brief show all sysfile name from /SPIFFS
 *
 * @return int 0 on success, -1 on failure,
 */
int show_file_spiffs_impl(void);

/**
 * @brief write log data to file.
 *
 * @param log_file_name Text file name to save log data.
 * @param log_data Log data.
 * @return int 0 on success, -1 on failure,
 */
int write_log_data_to_file_spiffs_impl(const char *log_file_name, const char *log_data);

#ifdef __cplusplus
}
#endif

#endif
