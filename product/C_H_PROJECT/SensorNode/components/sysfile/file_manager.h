
#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  char file_name[255];
  char file_size[20];
  void (*file_delete)(void *);
  uint8_t nfiles;
} file_data_ctx_t;

/**
 * @brief Initialize the file manager with the given partition and mount path.
 */
void fm_init(const char *partition_name, const char *root_path);

/**
 * @brief List all files in the specified path.
 */
void fm_file_list(const char *path);

/**
 * @brief Copy a file from source to destination.
 */
int fm_file_copy(const char *to, const char *from);

/**
 * @brief Print directory entry with indentation level.
 */
void fm_print_dir(char *direntName, int level);

/**
 * @brief Get the root path of the mounted filesystem.
 */
const char *fm_get_rootpath(void);

/**
 * @brief Extract filename from a full path.
 */
const char *fm_get_filename(const char *file);

/**
 * @brief Get the size of the specified file.
 */
size_t fm_get_file_size(const char *filepath);

/**
 * @brief Create a table of files with optional suffix filtering.
 */
int fm_file_table_create(char ***list_out, uint16_t *files_number, const char *filter_suffix);

/**
 * @brief Free memory allocated for the file table.
 */
int fm_file_table_free(char ***list, uint16_t files_number);

/**
 * @brief Read metadata or info about a file.
 */
void read_file_info(file_data_ctx_t *file_data);

/**
 * @brief Mark a file for deletion.
 */
void file_delete_set(file_data_ctx_t *file_data);

/**
 * @brief Prepare the file info table for sending logs to the web server.
 *
 * Reads stored log file metadata into a table structure,
 * enabling the system to manage and send logs to a web server.
 *
 * @param tb_p Pointer to the file data context table to populate.
 */
void read_file_info_table(file_data_ctx_t **tb_p);

#ifdef __cplusplus
}
#endif

#endif
