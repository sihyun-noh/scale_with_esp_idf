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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#include "log.h"
#include "sysfile.h"
#include "file_manager.h"

static const char *TAG = "file manager";

#define FILE_LEN_MAX 256

static char g_root_path[FILE_LEN_MAX];

// fnmatch defines
#define FNM_NOMATCH 1         // Match failed.
#define FNM_NOESCAPE 0x01     // Disable backslash escaping.
#define FNM_PATHNAME 0x02     // Slash must be matched by slash.
#define FNM_PERIOD 0x04       // Period must be matched by period.
#define FNM_LEADING_DIR 0x08  // Ignore /<tail> after Imatch.
#define FNM_CASEFOLD 0x10     // Case insensitive search.
#define FNM_PREFIX_DIRS 0x20  // Directory prefixes of pattern match too.
#define EOS '\0'

#define MAX_BUF (1024 * 50)

//-----------------------------------------------------------------------
static const char *rangematch(const char *pattern, char test, int flags) {
  int negate, ok;
  char c, c2;

  /*
   * A bracket expression starting with an unquoted circumflex
   * character produces unspecified results (IEEE 1003.2-1992,
   * 3.13.2).  This implementation treats it like '!', for
   * consistency with the regular expression syntax.
   * J.T. Conklin (conklin@ngai.kaleida.com)
   */
  if ((negate = (*pattern == '!' || *pattern == '^')))
    ++pattern;

  if (flags & FNM_CASEFOLD)
    test = tolower((unsigned char)test);

  for (ok = 0; (c = *pattern++) != ']';) {
    if (c == '\\' && !(flags & FNM_NOESCAPE))
      c = *pattern++;
    if (c == EOS)
      return (NULL);

    if (flags & FNM_CASEFOLD)
      c = tolower((unsigned char)c);

    if (*pattern == '-' && (c2 = *(pattern + 1)) != EOS && c2 != ']') {
      pattern += 2;
      if (c2 == '\\' && !(flags & FNM_NOESCAPE))
        c2 = *pattern++;
      if (c2 == EOS)
        return (NULL);

      if (flags & FNM_CASEFOLD)
        c2 = tolower((unsigned char)c2);

      if ((unsigned char)c <= (unsigned char)test && (unsigned char)test <= (unsigned char)c2)
        ok = 1;
    } else if (c == test)
      ok = 1;
  }
  return (ok == negate ? NULL : pattern);
}

//--------------------------------------------------------------------
static int fnmatch(const char *pattern, const char *string, int flags) {
  const char *stringstart;
  char c, test;

  for (stringstart = string;;)
    switch (c = *pattern++) {
      case EOS:
        if ((flags & FNM_LEADING_DIR) && *string == '/')
          return (0);
        return (*string == EOS ? 0 : FNM_NOMATCH);
      case '?':
        if (*string == EOS)
          return (FNM_NOMATCH);
        if (*string == '/' && (flags & FNM_PATHNAME))
          return (FNM_NOMATCH);
        if (*string == '.' && (flags & FNM_PERIOD) &&
            (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
          return (FNM_NOMATCH);
        ++string;
        break;
      case '*':
        c = *pattern;
        // Collapse multiple stars.
        while (c == '*')
          c = *++pattern;

        if (*string == '.' && (flags & FNM_PERIOD) &&
            (string == stringstart || ((flags & FNM_PATHNAME) && *(string - 1) == '/')))
          return (FNM_NOMATCH);

        // Optimize for pattern with * at end or before /.
        if (c == EOS)
          if (flags & FNM_PATHNAME)
            return ((flags & FNM_LEADING_DIR) || strchr(string, '/') == NULL ? 0 : FNM_NOMATCH);
          else
            return (0);
        else if ((c == '/') && (flags & FNM_PATHNAME)) {
          if ((string = strchr(string, '/')) == NULL)
            return (FNM_NOMATCH);
          break;
        }

        // General case, use recursion.
        while ((test = *string) != EOS) {
          if (!fnmatch(pattern, string, flags & ~FNM_PERIOD))
            return (0);
          if ((test == '/') && (flags & FNM_PATHNAME))
            break;
          ++string;
        }
        return (FNM_NOMATCH);
      case '[':
        if (*string == EOS)
          return (FNM_NOMATCH);
        if ((*string == '/') && (flags & FNM_PATHNAME))
          return (FNM_NOMATCH);
        if ((pattern = rangematch(pattern, *string, flags)) == NULL)
          return (FNM_NOMATCH);
        ++string;
        break;
      case '\\':
        if (!(flags & FNM_NOESCAPE)) {
          if ((c = *pattern++) == EOS) {
            c = '\\';
            --pattern;
          }
        }
        break;
        // FALLTHROUGH
      default:
        if (c == *string) {
        } else if ((flags & FNM_CASEFOLD) && (tolower((unsigned char)c) == tolower((unsigned char)*string))) {
        } else if ((flags & FNM_PREFIX_DIRS) && *string == EOS &&
                   ((c == '/' && string != stringstart) || (string == stringstart + 1 && *stringstart == '/')))
          return (0);
        else
          return (FNM_NOMATCH);
        string++;
        break;
    }
  // NOTREACHED
  return 0;
}

static void traverse_dir(char *direntName, int level, int indent) {
  DIR *p_dir = NULL;
  struct dirent *p_dirent = NULL;

  p_dir = opendir(direntName);

  if (p_dir == NULL) {
    printf("opendir error\n");
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
    char *backupDirName = NULL;

    if (p_dirent->d_name[0] == '.') {
      continue;
    }

    int i;

    for (i = 0; i < indent; i++) {
      // printf("|");
      printf("     ");
    }

    printf("|--- %s", p_dirent->d_name);

    /* Itme is a file */
    if (p_dirent->d_type == DT_REG) {
      int curDirentNameLen = strlen(direntName) + strlen(p_dirent->d_name) + 2;

      // prepare next path
      backupDirName = (char *)malloc(curDirentNameLen);
      struct stat *st = NULL;
      st = malloc(sizeof(struct stat));

      if (NULL == backupDirName || NULL == st) {
        goto _err;
      }

      memset(backupDirName, 0, curDirentNameLen);
      memcpy(backupDirName, direntName, strlen(direntName));

      strcat(backupDirName, "/");
      strcat(backupDirName, p_dirent->d_name);

      int statok = stat(backupDirName, st);

      if (0 == statok) {
        printf("[%dB]\n", (int)st->st_size);
      } else {
        printf("\n");
      }

      free(backupDirName);
      backupDirName = NULL;
    } else {
      printf("\n");
    }

    /* Itme is a directory */
    if (p_dirent->d_type == DT_DIR) {
      int curDirentNameLen = strlen(direntName) + strlen(p_dirent->d_name) + 2;

      // prepare next path
      backupDirName = (char *)malloc(curDirentNameLen);

      if (NULL == backupDirName) {
        goto _err;
      }

      memset(backupDirName, 0, curDirentNameLen);
      memcpy(backupDirName, direntName, curDirentNameLen);

      strcat(backupDirName, "/");
      strcat(backupDirName, p_dirent->d_name);

      if (level > 0) {
        traverse_dir(backupDirName, level - 1, indent + 1);
      }

      free(backupDirName);
      backupDirName = NULL;
    }
  }

_err:
  closedir(p_dir);
}

//-----------------------------------------
static void file_list(const char *path, char *match) {
  DIR *dir = NULL;
  struct dirent *ent;
  char type;
  char size[20];
  char tpath[255];
  char tbuffer[80];
  struct stat sb;
  struct tm *tm_info;
  char *lpath = NULL;
  int statok;

  printf("\nList of Directory [%s]\n", path);
  printf("-----------------------------------\n");
  // Open directory
  dir = opendir(path);
  if (!dir) {
    printf("Error opening directory\n");
    return;
  }

  // Read directory entries
  uint64_t total = 0;
  int nfiles = 0;
  printf("T  Size      Date/Time         Name\n");
  printf("-----------------------------------\n");
  while ((ent = readdir(dir)) != NULL) {
    sprintf(tpath, path);
    if (path[strlen(path) - 1] != '/')
      strcat(tpath, "/");
    strcat(tpath, ent->d_name);
    tbuffer[0] = '\0';

    if ((match == NULL) || (fnmatch(match, tpath, (FNM_PERIOD)) == 0)) {
      // Get file stat
      statok = stat(tpath, &sb);

      if (statok == 0) {
        tm_info = localtime(&sb.st_mtime);
        strftime(tbuffer, 80, "%d/%m/%Y %R", tm_info);
      } else
        sprintf(tbuffer, "                ");

      if (ent->d_type == DT_REG) {
        type = 'f';
        nfiles++;
        if (statok)
          strcpy(size, "       ?");
        else {
          total += sb.st_size;
          if (sb.st_size < (1024 * 1024))
            sprintf(size, "%8d", (int)sb.st_size);
          else if ((sb.st_size / 1024) < (1024 * 1024))
            sprintf(size, "%6dKB", (int)(sb.st_size / 1024));
          else
            sprintf(size, "%6dMB", (int)(sb.st_size / (1024 * 1024)));
        }
      } else {
        type = 'd';
        strcpy(size, "       -");
      }

      printf("%c  %s  %s  %s\r\n", type, size, tbuffer, ent->d_name);
    }
  }
  if (total) {
    printf("-----------------------------------\n");
    if (total < (1024 * 1024))
      printf("   %8d", (int)total);
    else if ((total / 1024) < (1024 * 1024))
      printf("   %6dKB", (int)(total / 1024));
    else
      printf("   %6dMB", (int)(total / (1024 * 1024)));
    printf(" in %d file(s)\n", nfiles);
  }
  printf("-----------------------------------\n");

  closedir(dir);

  free(lpath);

  // uint32_t tot = 0, used = 0;
  // esp_spiffs_info(NULL, &tot, &used);
  // printf("SPIFFS: free %d KB of %ld KB\n", (long unsigend int)(tot - used) / 1024, (long unsigend int)tot / 1024);
  printf("-----------------------------------\n\n");
}

void fm_init(const char *partition_name, const char *root_path) {
  strcpy(g_root_path, root_path);

  init_sysfile(partition_name, root_path);
  fm_file_list(root_path);
}

const char *fm_get_rootpath(void) {
  return g_root_path;
}

void fm_file_list(const char *path) {
  file_list(path, NULL);
}

void fm_print_dir(char *direntName, int level) {
  printf("Traverse directory %s\n", direntName);
  traverse_dir(direntName, level, 0);
  printf("\r\n");
}

int fm_file_table_create(char ***list_out, uint16_t *files_number, const char *filter_suffix) {
  DIR *p_dir = NULL;
  struct dirent *p_dirent = NULL;

  LOGI(TAG, "g_root_path = %s", g_root_path);

  p_dir = opendir(g_root_path);

  if (p_dir == NULL) {
    LOGE(TAG, "opendir error");
    return -1;
  }

  uint16_t f_num = 0;
  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (p_dirent->d_type == DT_REG) {
      f_num++;
    }
  }

  LOGI(TAG, "file number = %d", f_num);
  rewinddir(p_dir);

  *list_out = calloc(f_num, sizeof(char *));
  if (NULL == (*list_out)) {
    goto _err;
  }
  for (size_t i = 0; i < f_num; i++) {
    (*list_out)[i] = malloc(FILE_LEN_MAX);
    if (NULL == (*list_out)[i]) {
      LOGE(TAG, "malloc failed at %d", i);
      fm_file_table_free(list_out, f_num);
      goto _err;
    }
  }

  uint16_t index = 0;
  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (p_dirent->d_type == DT_REG) {
      if (NULL != filter_suffix) {
        if (strstr(p_dirent->d_name, filter_suffix)) {
          strncpy((*list_out)[index], p_dirent->d_name, FILE_LEN_MAX - 1);
          (*list_out)[index][FILE_LEN_MAX - 1] = '\0';
          index++;
        }
      } else {
        strncpy((*list_out)[index], p_dirent->d_name, FILE_LEN_MAX - 1);
        (*list_out)[index][FILE_LEN_MAX - 1] = '\0';
        index++;
      }
    }
  }
  (*files_number) = index;

  closedir(p_dir);
  return 0;
_err:
  closedir(p_dir);

  return -1;
}

//----------------------------------------------------
int fm_file_copy(const char *to, const char *from) {
  FILE *fd_to;
  FILE *fd_from;
  char buf[1024];
  ssize_t nread;
  int saved_errno;

  fd_from = fopen(from, "rb");
  // fd_from = open(from, O_RDONLY);
  if (fd_from == NULL) {
    printf("Failed to read from = %s\n", from);
    return -1;
  }

  fd_to = fopen(to, "wb");
  if (fd_to == NULL) {
    printf("Failed to write for open from = %s\n", to);
    goto out_error;
  }

  while ((nread = fread(buf, 1, sizeof(buf), fd_from)) > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;

    printf("nread = %d\n", nread);

    do {
      nwritten = fwrite(out_ptr, 1, nread, fd_to);
      printf("nwritten = %d\n", nwritten);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  printf("nread1 = %d\n", nread);
  if (nread == 0) {
    if (fclose(fd_to) < 0) {
      fd_to = NULL;
      goto out_error;
    }
    fclose(fd_from);

    printf("file copy success!!! from : %s to %s\n", from, to);
    // Success!
    return 0;
  }

out_error:
  saved_errno = errno;

  printf("errno = %d\n", errno);

  fclose(fd_from);
  if (fd_to)
    fclose(fd_to);

  printf("file copy failure!!! from : %s to %s\n", from, to);
  errno = saved_errno;
  return -1;
}

int fm_file_table_free(char ***list, uint16_t files_number) {
  for (size_t i = 0; i < files_number; i++) {
    free((*list)[i]);
  }
  free((*list));
  return 0;
}

const char *fm_get_filename(const char *file) {
  const char *p = file + strlen(file);
  while (p > file) {
    if ('/' == *p) {
      return (p + 1);
    }
    p--;
  }
  return file;
}

size_t fm_get_file_size(const char *filepath) {
  struct stat siz = { 0 };
  stat(filepath, &siz);
  return siz.st_size;
}

int fm_mkdir(const char *path) {
  struct stat st;
  int status = 0;

  if (stat(path, &st) != 0) {
    // Directory does not exist. EEXIST for race condition
    LOGI(TAG, "Create dir [%s]", path);
    if (mkdir(path, 0755) != 0 && errno != EEXIST) {
      status = -1;
      LOGE(TAG, "Create dir [%s] failed", path);
    }
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    status = -1;
    LOGE(TAG, "Exist [%s] but not dir", path);
  }

  return status;
}
