#ifndef _LITTLEFS_IMPL_
#define _LITTLEFS_IMPL_

#define BASE_PATH      "/storage"
#define PARTITION_NAME "storage"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the LittleFS filesystem on the specified partition.
 *
 * @param partition_name Name of the partition to mount LittleFS on.
 * @param root_path Mount point path in the virtual file system.
 * @return 0 on success, non-zero error code on failure.
 */
int init_littlefs_impl(const char *partition_name, const char *root_path);

/**
 * @brief Format the LittleFS partition.
 *
 * Erases and formats the entire LittleFS partition.
 * @return 0 on success, non-zero error code on failure.
 */
int format_littlefs_impl(void);

/**
 * @brief List and display files in the LittleFS partition.
 *
 * Prints the file names and metadata stored in the mounted LittleFS partition.
 * @return 0 on success, non-zero error code on failure.
 */
int show_file_littlefs_impl(void);

/**
 * @brief Write log data to a specified file in LittleFS.
 *
 * Opens (or creates) the log file and appends the given log data.
 *
 * @param log_file_name Name of the file to write log data to.
 * @param log_data String data to append to the log file.
 * @return 0 on success, non-zero error code on failure.
 */
int write_log_data_to_file_littlefs_impl(const char *log_file_name, const char *log_data);

#ifdef __cplusplus
}
#endif

#endif /* _LITTLEFS_IMPL_ */
