/**
 * @file filelog.c
 *
 * @brief It is a program that manages the creation and deletion of log files.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/unistd.h>
#include <time.h>

#include "esp_log.h"

#include "file_log.h"
#include "sys_file.h"
#include "sdkconfig.h"

#define FILE_LOG_MAX_BUFF_SIZE 1024
#define FILE_LOG_MAX_MSG_SIZE  (FILE_LOG_MAX_BUFF_SIZE)
// #define FILE_LOG_MAX_FILE_SIZE (FILE_LOG_MAX_MSG_SIZE * 500)
#define FILE_LOG_MAX_FILE_SIZE (FILE_LOG_MAX_MSG_SIZE * 1)
#define DIR_COUNT              3
#define FILE_COUNT             3
#define LOG_TIMESTAMP_SIZE     64

static const char *TAG = "FILE_LOG";

int g_file_log_num = FILE_COUNT;

typedef struct filelog {
  uint8_t file_num;
  size_t latest_file_size;
  char latest_file[30];
  char oldest_file[30];
  char filepath[50];
} file_log_t;

static file_log_t file_ctx;

char *log_timestamp(void) {
  static char timestamp[LOG_TIMESTAMP_SIZE];
  static _lock_t bufferLock = 0;

  time_t now = time(NULL);
  struct tm *tm = localtime(&now);

  _lock_acquire(&bufferLock);
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm);
  _lock_release(&bufferLock);

  return timestamp;
}

/* Check the file name, number of files, latest files, and oldest files in the file directory. */

static int file_status_check(const char *dirpath) {
  uint64_t latest_cmp_date = 0;
  uint64_t oldest_cmp_date = 0;
  uint64_t temp_date = 0;
  char filepath[50] = { 0 };
  char temp_buf[50] = { 0 };
  char temp_date_buf[20] = { 0 };
  uint8_t file_count = 0;

  struct dirent *entry;
  struct stat file_stat;

  DIR *dir = opendir(dirpath);

  if (!dir) {
    ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
    return -1;
  }

  while ((entry = readdir(dir)) != NULL) {
    strlcpy(temp_buf, entry->d_name, strlen(entry->d_name) + 1);

    memset(temp_date_buf, 0, sizeof(temp_date_buf));
    char *tok = strtok(temp_buf, "_");

    char *tok_2 = strtok(tok, "-");
    while (tok_2 != NULL) {
      strncat(temp_date_buf, tok_2, strlen(tok_2));
      tok_2 = strtok(NULL, "-");
    }

    temp_date = atoll(temp_date_buf);

    if (oldest_cmp_date == 0 && oldest_cmp_date == 0) {
      oldest_cmp_date = temp_date;
      latest_cmp_date = temp_date;
    }

    if (temp_date <= oldest_cmp_date) {
      oldest_cmp_date = temp_date;
      strlcpy(file_ctx.oldest_file, entry->d_name, strlen(entry->d_name) + 1);
    }

    if (temp_date >= latest_cmp_date) {
      latest_cmp_date = temp_date;
      strlcpy(file_ctx.latest_file, entry->d_name, strlen(entry->d_name) + 1);

      strlcpy(filepath, dirpath, strlen(dirpath) + 1);
      strncat(filepath, "/", 2);
      strncat(filepath, entry->d_name, strlen(entry->d_name));
      // ESP_LOGE(TAG, "filepath : %s", filepath);

      if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "stat error. file path : %s", filepath);
      } else {
        file_ctx.latest_file_size = file_stat.st_size;
        strlcpy(file_ctx.filepath, filepath, strlen(filepath) + 1);
      }
    }
    file_count++;
  }
  file_ctx.file_num = file_count;
  // ESP_LOGI(TAG, "latest file : %s", file_ctx.latest_file);
  // ESP_LOGI(TAG, "oldest file : %s", file_ctx.oldest_file);
  // ESP_LOGI(TAG, "file_num : %d", file_ctx.file_num);
  // ESP_LOGI(TAG, "file_path : %s", file_ctx.filepath);

  closedir(dir);
  return 0;
}

/* Extracts the elements necessary to create a file name and
 * combines them to create a file name. */
// must be free newfilepath
static char *file_path_name(const char *path) {
  char temp_buf[50] = { 0 };
  char time_date_buf[30] = { 0 };
  char *time_date;
  char *newfilepath;

  newfilepath = malloc(sizeof(temp_buf));
  time_date = log_timestamp();
  strlcpy(time_date_buf, time_date, 17);  // 2022-00-00 10:20

  char *tok = strtok(time_date_buf, ":");  // 2022-00-00 10
  char *tok_1 = strtok(NULL, ":");         // 20
  char *tok_2 = strtok(tok, " ");          // 2022-00-00
  char *tok_3 = strtok(NULL, " ");         // 10

  memset(temp_buf, 0, sizeof(temp_buf));
  strlcpy(temp_buf, path, strlen(path) + 1);
  strncat(temp_buf, "/", 2);
  strncat(temp_buf, tok_2, strlen(tok_2));
  strncat(temp_buf, "-", 2);
  strncat(temp_buf, tok_3, strlen(tok_3));
  strncat(temp_buf, tok_1, strlen(tok_1));
  strncat(temp_buf, "_log.txt", 9);
  strlcpy(newfilepath, temp_buf, strlen(temp_buf) + 1);

  return newfilepath;
}

/**
 * @brief Extracts the sequence number from a filename in the format: "xxxx[NNN]_log.txt"
 *
 * This function locates the numeric sequence enclosed in square brackets (e.g., [001]) and
 * parses it as an integer.
 *
 * @param[in]  filename   The name of the file (e.g., "2025-05-23-1120[001]_log.txt")
 * @param[out] seq_num    Pointer to store the extracted integer sequence number
 * @return 0 on success, -1 on parsing error or malformed filename
 */
static int extract_seq_num(const char *filename, int *seq_num) {
  const char *left = strchr(filename, '[');
  const char *right = strchr(filename, ']');
  char buf[8] = { 0 };

  if (!left || !right || right <= left + 1)
    return -1;
  size_t len = right - left - 1;
  if (len >= sizeof(buf))
    return -1;

  strncpy(buf, left + 1, len);
  buf[len] = '\0';

  *seq_num = atoi(buf);
  return 0;
}

/**
 * @brief Scans the specified directory to determine the oldest and latest files based on sequence numbers.
 *
 * This function iterates through all files in the directory, extracts sequence numbers using `extract_seq_num()`,
 * and identifies the files with the smallest and largest sequence numbers respectively.
 * The results (file names, size, and path of the latest file) are stored in a global `file_ctx` structure.
 *
 * @param[in] dirpath  Path to the directory containing log files
 * @return 0 on success, -1 if directory cannot be opened or no valid files found
 */
static int new_file_status_check(const char *dirpath) {
  int min_seq = 0;
  int max_seq = -1;
  char filepath[300] = { 0 };
  uint8_t file_count = 0;

  struct dirent *entry;
  struct stat file_stat;
  DIR *dir = opendir(dirpath);
  if (!dir)
    return -1;

  while ((entry = readdir(dir)) != NULL) {
    int seq_num = 0;
    if (extract_seq_num(entry->d_name, &seq_num) != 0)
      continue;

    // oldest
    if (seq_num < min_seq) {
      min_seq = seq_num;
      strlcpy(file_ctx.oldest_file, entry->d_name, sizeof(file_ctx.oldest_file));
    }

    // latest
    if (seq_num > max_seq) {
      max_seq = seq_num;
      strlcpy(file_ctx.latest_file, entry->d_name, sizeof(file_ctx.latest_file));
      snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);
      if (stat(filepath, &file_stat) == 0) {
        file_ctx.latest_file_size = file_stat.st_size;
        strlcpy(file_ctx.filepath, filepath, sizeof(file_ctx.filepath));
      }
    }

    file_count++;
  }

  file_ctx.file_num = file_count;
  closedir(dir);
  return 0;
}

/**
 * @brief Generates a new file path string with a timestamp and a formatted sequence number.
 *
 * This function creates a filename in the format: "YYYY-MM-DD-HHMM[NNN]_log.txt"
 * where the timestamp is derived from `log_timestamp()` and the sequence number is passed as a parameter.
 *
 * @param[in]  base_path   Directory path (e.g., "/log")
 * @param[in]  number      Sequence number to embed in the file name (e.g., 1 → [001])
 * @return Pointer to a dynamically allocated string containing the full file path.
 *         Caller is responsible for freeing the returned string.
 */
static char *new_file_path_name(const char *path, int number) {
  char temp_buf[300] = { 0 };  // 크기 증가
  char time_date_buf[30] = { 0 };
  char number_buf[8] = { 0 };
  char *time_date;
  char *newfilepath;

  newfilepath = malloc(sizeof(char) * 300);  // 여유 있게 할당
  if (!newfilepath)
    return NULL;

  time_date = log_timestamp();  // 예: "2025-05-23 10:20"
  strlcpy(time_date_buf, time_date, 17);

  char *tok = strtok(time_date_buf, ":");  // "2025-05-23 10"
  char *tok_1 = strtok(NULL, ":");         // "20"
  char *tok_2 = strtok(tok, " ");          // "2025-05-23"
  char *tok_3 = strtok(NULL, " ");         // "10"

  snprintf(number_buf, sizeof(number_buf), "[%03d]", number);  // "[001]"

  strlcpy(temp_buf, path, sizeof(temp_buf));
  strncat(temp_buf, "/", sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, tok_2, sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, "-", sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, tok_3, sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, tok_1, sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, number_buf, sizeof(temp_buf) - strlen(temp_buf) - 1);
  strncat(temp_buf, "_log.txt", sizeof(temp_buf) - strlen(temp_buf) - 1);

  strlcpy(newfilepath, temp_buf, 300);
  return newfilepath;
}

/* When the number of files exceeds 10, Delete oldest files. */
static int file_delete(char *file_delete_path, const char *path) {
  const char *dirpath = path;
  char temp_buf[50] = { 0 };
  uint8_t pathlen;

  strncat(temp_buf, dirpath, strlen(dirpath) + 1);
  strncat(temp_buf, "/", 2);
  strncat(temp_buf, file_delete_path, strlen(file_delete_path));

  pathlen = strlen(temp_buf);
  strlcpy(temp_buf, temp_buf, pathlen + 1);
  ESP_LOGW(TAG, "Target file : %s", temp_buf);

  return unlink(temp_buf);
}

/*The system file log is saved in the file storage device using the "FLOG" macro.*/
int file_log_write(char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  ret = file_status_check(BASE_PATH);
  if (ret != 0) {
    ESP_LOGE(TAG, "file status check fail.");
    return -1;
  }

  if (file_ctx.file_num == 0) {
    newfilepath = file_path_name(BASE_PATH);
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;
    newfilepath = file_path_name(BASE_PATH);
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    if (file_delete(file_ctx.oldest_file, BASE_PATH) != 0) {
      ESP_LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

#define CONFIG_SEQUENCE_FILE_NUMBERING

/*The system file log is saved in the file storage device using the "FLOG" macro.*/
int file_log_write_datalogger(char *path, char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

#ifdef CONFIG_SEQUENCE_FILE_NUMBERING
  ret = new_file_status_check(path);
  if (ret != 0) {
    ESP_LOGE(TAG, "file status check fail.");
    return -1;
  }

#else
  ret = file_status_check(path);
  if (ret != 0) {
    ESP_LOGE(TAG, "file status check fail.");
    return -1;
  }
#endif

  ESP_LOGE(TAG, " file count : %d", file_ctx.file_num);
  ESP_LOGI(TAG, " path : %s", path);
  ESP_LOGI(TAG, " buff : %s", buff);

  if (file_ctx.file_num == 0) {
#ifdef CONFIG_SEQUENCE_FILE_NUMBERING
    newfilepath = new_file_path_name(path, file_ctx.file_num + 1);
#else
    newfilepath = file_path_name(path);
#endif
    ESP_LOGW(TAG, "filename : %s", newfilepath);

#if (CONFIG_BS_PLATFORM_GASSENSOR)
    char index_buf[60] = "TIME,NH3,CO,H2S,O2,CH4,S_TEMP,S_MOS,S_EC,A_TEMP,A_MOS\n";
    write_log(newfilepath, index_buf);
#endif
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  ESP_LOGW(TAG, "latest_file size %d / %d ", (int)file_ctx.latest_file_size, FILE_LOG_MAX_FILE_SIZE);

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;

#ifdef CONFIG_SEQUENCE_FILE_NUMBERING
    newfilepath = new_file_path_name(path, file_ctx.file_num + 1);
#else
    newfilepath = file_path_name(path);
#endif
#if (CONFIG_BS_PLATFORM_GASSENSOR)
    char index_buf[60] = "TIME,NH3,CO,H2S,O2,CH4,S_TEMP,S_MOS,S_EC,A_TEMP,A_MOS\n";
    write_log(newfilepath, index_buf);
#endif
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    ESP_LOGE(TAG, "file delete : %s", file_ctx.oldest_file);
    if (file_delete(file_ctx.oldest_file, path) != 0) {
      ESP_LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

/*The system file log is saved in the file storage device using the "FLOG" macro.*/
int new_file_log_write_datalogger(const char *table_index, char *path, char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  ret = file_status_check(path);
  if (ret != 0) {
    ESP_LOGE(TAG, "file status check fail.");
    return -1;
  }

  ESP_LOGE(TAG, " file count : %d", file_ctx.file_num);
  ESP_LOGI(TAG, " path : %s", path);
  ESP_LOGI(TAG, " buff : %s", buff);

  if (file_ctx.file_num == 0) {
    newfilepath = file_path_name(path);
    write_log(newfilepath, table_index);
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;
    newfilepath = file_path_name(path);
    write_log(newfilepath, table_index);
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    ESP_LOGE(TAG, "file delete!");
    if (file_delete(file_ctx.oldest_file, path) != 0) {
      ESP_LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

int new_file_log_write_judge(const char *table_index, char *path, char *format, ...) {
  char *newfilepath;
  int ret = 0;
  char buff[FILE_LOG_MAX_MSG_SIZE] = { 0 };

  // memcpy(buff, format, strlen(format));
  va_list list;
  va_start(list, format);
  vsnprintf(buff, sizeof(buff), format, list);
  va_end(list);

  ret = file_status_check(path);
  if (ret != 0) {
    ESP_LOGE(TAG, "file status check fail.");
    return -1;
  }

  ESP_LOGE(TAG, " file count : %d", file_ctx.file_num);
  ESP_LOGI(TAG, " path : %s", path);
  ESP_LOGI(TAG, " buff : %s", buff);

  if (file_ctx.file_num == 0) {
    newfilepath = file_path_name(path);
    write_log(newfilepath, table_index);
    write_log(newfilepath, buff);
    free(newfilepath);
    goto END;
  }

  if (file_ctx.latest_file_size >= FILE_LOG_MAX_FILE_SIZE) {
    file_ctx.latest_file_size = 0;
    newfilepath = file_path_name(path);
    write_log(newfilepath, table_index);
    write_log(newfilepath, buff);
    free(newfilepath);
  } else {
    write_log(file_ctx.filepath, buff);
  }

  if (file_ctx.file_num > g_file_log_num) {
    ESP_LOGE(TAG, "file delete!");
    if (file_delete(file_ctx.oldest_file, path) != 0) {
      ESP_LOGE(TAG, "file delete erorr!");
    }
  }
END:
  return 0;
}

void set_file_log_number(int file_log_num) {
  g_file_log_num = file_log_num;
}
