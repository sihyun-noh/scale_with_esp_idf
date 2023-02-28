/**
 * @file filelog.c
 *
 * @brief It is a program that manages the creation and deletion of log files.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef _FILELOG_H_
#define _FILELOG_H_

#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Used to write the system log to a file.
 *
 * @param format format of the output log. see ``printf``
 * @param ... variables to be replaced into the log. see ``printf``
 * @return int 0 on success, -1 on failure,
 */
int file_log_write(char *format, ...);

int file_log_write_datalogger(char *path, char *format, ...);

void set_file_log_number(int file_log_num);

#define FILE_LOG_FORMAT(letter, format) #letter " (%s) %s: " format "\r\n"
#define FILE_LOG_FORMAT_FOR_DATALOGGER(format) "%s," format "\n"

#define FILELOGI(tag, format, ...) FILE_LOG_LEVEL(LOG_INFO, tag, format, ##__VA_ARGS__)
#define FILELOGW(tag, format, ...) FILE_LOG_LEVEL(LOG_WARN, tag, format, ##__VA_ARGS__)
#define FILELOGE(tag, format, ...) FILE_LOG_LEVEL(LOG_ERROR, tag, format, ##__VA_ARGS__)

#define FILE_LOG_LEVEL(level, tag, format, ...)                                        \
  do {                                                                                 \
    if (level == LOG_ERROR) {                                                          \
      file_log_write(FILE_LOG_FORMAT(E, format), log_timestamp(), tag, ##__VA_ARGS__); \
    } else if (level == LOG_WARN) {                                                    \
      file_log_write(FILE_LOG_FORMAT(W, format), log_timestamp(), tag, ##__VA_ARGS__); \
    } else {                                                                           \
      file_log_write(FILE_LOG_FORMAT(I, format), log_timestamp(), tag, ##__VA_ARGS__); \
    }                                                                                  \
  } while (0)

#define FLOGI(tag, format, ...)           \
  do {                                    \
    FILELOGI(tag, format, ##__VA_ARGS__); \
  } while (0)

#define FLOGW(tag, format, ...)           \
  do {                                    \
    FILELOGW(tag, format, ##__VA_ARGS__); \
  } while (0)

#define FLOGE(tag, format, ...)           \
  do {                                    \
    FILELOGE(tag, format, ##__VA_ARGS__); \
  } while (0)

#define FDATA(path, format, ...)                                                                             \
  do {                                                                                                       \
    file_log_write_datalogger(path, FILE_LOG_FORMAT_FOR_DATALOGGER(format), log_timestamp(), ##__VA_ARGS__); \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif /* _FILELOG_H_ */
