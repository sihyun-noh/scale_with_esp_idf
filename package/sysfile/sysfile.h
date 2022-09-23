#ifndef _SYS_FILE_IMPL_H_
#define _SYS_FILE_IMPL_H_

#include "esp32/spiffs_impl.h"

/**
 * @brief Register and mount SPIFFS to VFS with given path prefix.
 *
 * @return int 0 on success, -1 on failure,
 */
int init_sysfile(void);

/**
 * @brief Format the SPIFFS partition
 *
 * @return int 0 on success, -1 on failure,
 */
int sysfile_format(void);

/**
 * @brief show all sysfile name form /SPIFFS
 *
 * @return int 0 on success, -1 on failure,
 */

int sysfile_show_file(void);

/**
 * @brief write log data to file.
 *
 * @param log_file_name Text file name to save log data.
 * @param log_data Log data.
 * @return int 0 on success, -1 on failure,
 */
int write_log(const char *log_file_name, const char *log_data);

#endif
