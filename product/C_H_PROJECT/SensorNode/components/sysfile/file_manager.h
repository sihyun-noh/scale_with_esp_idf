
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

void fm_init(const char *partition_name, const char *root_path);
void fm_file_list(const char *path);
int fm_file_copy(const char *to, const char *from);
void fm_print_dir(char *direntName, int level);
const char *fm_get_rootpath(void);
const char *fm_get_filename(const char *file);
size_t fm_get_file_size(const char *filepath);
int fm_file_table_create(char ***list_out, uint16_t *files_number, const char *filter_suffix);
int fm_file_table_free(char ***list, uint16_t files_number);
void read_file_info(file_data_ctx_t *file_data);
void file_delete_set(file_data_ctx_t *file_data);
void read_file_info_table(file_data_ctx_t **tb_p);
#ifdef __cplusplus
}
#endif

#endif
