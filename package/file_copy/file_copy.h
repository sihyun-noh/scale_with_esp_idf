#ifndef _FILE_COPY_H_
#define _FILE_COPY_H_

#ifdef __cplusplus
extern "C" {
#endif

int file_copy(const char *to, const char *from);
void file_list(const char *path);
void readTest(char *fname);
int get_file_list_cmd(int argc, char **argv);
int get_file_read_cmd(int argc, char **argv);
int make_dir(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* _FILE_COPY_H_ */
