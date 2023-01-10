// Copyright 2015-2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _FILE_MANAGER_H_
#define _FILE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

void fm_init(const char *partition_name, const char *root_path);
void fm_file_list(const char *path);
int fm_file_copy(const char *to, const char *from);
void fm_print_dir(char *direntName, int level);
const char *fm_get_rootpath(void);
const char *fm_get_filename(const char *file);
size_t fm_get_file_size(const char *filepath);
int fm_file_table_create(char ***list_out, uint16_t *files_number, const char *filter_suffix);
int fm_file_table_free(char ***list, uint16_t files_number);

#ifdef __cplusplus
}
#endif

#endif
