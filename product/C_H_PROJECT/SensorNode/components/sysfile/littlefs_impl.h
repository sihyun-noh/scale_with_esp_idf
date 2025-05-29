#ifndef _LITTLEFS_IMPL_
#define _LITTLEFS_IMPL_

#define BASE_PATH "/storage"
#define PARTITION_NAME "storage"

#ifdef __cplusplus
extern "C" {
#endif

int init_littlefs_impl(const char *partition_name, const char *root_path);

int format_littlefs_impl(void);

int show_file_littlefs_impl(void);

int write_log_data_to_file_littlefs_impl(const char *log_file_name, const char *log_data);

#ifdef __cplusplus
}
#endif

#endif /* _LITTLEFS_IMPL_ */
