
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <ctype.h>

#include "esp_vfs.h"
#include "esp_log.h"
#include <esp_vfs_fat.h>
#include "msc_host_vfs.h"
#include "ffconf.h"
#include "file_copy.h"

#include "syslog.h"

static const char tag[] = "[SPIFFS example]";

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

// ============================================================================

//-----------------------------------------
static void list(char *path, char *match) {
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

void file_list(const char *path) {
  list(path, NULL);
}

//----------------------------------------------------
int file_copy(const char *to, const char *from) {
  FILE *fd_to;
  FILE *fd_from;
  char buf[1024];
  ssize_t nread;
  int saved_errno;

  fd_from = fopen(from, "rb");
  // fd_from = open(from, O_RDONLY);
  if (fd_from == NULL)
    return -1;

  fd_to = fopen(to, "wb");
  if (fd_to == NULL)
    goto out_error;

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

//--------------------------------
static void writeTest(char *fname) {
  printf("=======================\n");
  printf("==== Write to file ====\n");
  printf("=======================\n");
  printf("  file: \"%s\"\n", fname);

  int n, res, tot, len;
  char buf[40];

  FILE *fd = fopen(fname, "wb");
  if (fd == NULL) {
    printf("  Error opening file (%d) %s\n", errno, strerror(errno));
    printf("\n");
    return;
  }
  tot = 0;
  for (n = 1; n < 11; n++) {
    sprintf(buf, "ESP32 spiffs write to file, line %d\n", n);
    len = strlen(buf);
    res = fwrite(buf, 1, len, fd);
    if (res != len) {
      printf("  Error writing to file(%d <> %d\n", res, len);
      break;
    }
    tot += res;
  }
  printf("  %d bytes written\n", tot);
  res = fclose(fd);
  if (res) {
    printf("     Error closing file\r\n");
  }
  printf("\n");
}

//-------------------------------
void readTest(char *fname) {
  printf("  file: \"%s\"\n", fname);

  int res;
  char *buf;
  // buf = calloc(1024, 1);
  buf = calloc(MAX_BUF, 1);
  if (buf == NULL) {
    printf("  Error allocating read buffer\n");
    printf("\n");
    return;
  }

  FILE *fd = fopen(fname, "rb");
  if (fd == NULL) {
    printf("  Error opening file (%d) %s\n", errno, strerror(errno));
    free(buf);
    printf("\n");
    return;
  }
  res = 999;
  res = fread(buf, 1, MAX_BUF, fd);
  if (res <= 0) {
    printf("  Error reading from file\n");
  } else {
    printf("  %d bytes read [\n", res);
    buf[res] = '\0';
    printf("%s\n]\n", buf);
  }
  free(buf);

  res = fclose(fd);
  if (res) {
    printf("  Error closing file\n");
  }
  printf("\n");
}

//----------------------------------
static void mkdirTest(char *dirname) {  // spiffs not support
  printf("============================\n");
  printf("==== Make new directory ====\n");
  printf("============================\n");
  printf("  dir: \"%s\"\n", dirname);

  int res;
  struct stat st = { 0 };
  char nname[80];

  if (stat(dirname, &st) == -1) {
    res = mkdir(dirname, 0777);
    if (res != 0) {
      printf("  Error creating directory (%d) %s\n", errno, strerror(errno));
      printf("\n");
      return;
    }
    printf("  Directory created\n");

    printf("  Copy file from root to new directory...\n");
    sprintf(nname, "%s/test.txt.copy", dirname);
    res = file_copy(nname, "/spiffs/test.txt");
    if (res != 0) {
      printf("  Error copying file (%d)\n", res);
    }

    printf("  List the new directory\n");
    list(dirname, NULL);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    printf("  List root directory, the \"newdir\" should be listed\n");
    list("/spiffs/", NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("  Try to remove non empty directory...\n");
    res = rmdir(dirname);
    if (res != 0) {
      printf("  Error removing directory (%d) %s\n", errno, strerror(errno));
    }

    printf("  Removing file from new directory...\n");
    res = remove(nname);
    if (res != 0) {
      printf("  Error removing file (%d) %s\n", errno, strerror(errno));
    }

    printf("  Removing directory...\n");
    res = rmdir(dirname);
    if (res != 0) {
      printf("  Error removing directory (%d) %s\n", errno, strerror(errno));
    }

    printf("  List root directory, the \"newdir\" should be gone\n");
    list("/spiffs/", NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  } else {
    printf("  Directory already exists, removing\n");
    res = rmdir(dirname);
    if (res != 0) {
      printf("  Error removing directory (%d) %s\n", errno, strerror(errno));
    }
  }

  printf("\n");
}

//------------------------------------------------------
static int writeFile(char *fname, char *mode, char *buf) {
  FILE *fd = fopen(fname, mode);
  if (fd == NULL) {
    LOGE("[write]", "fopen failed");
    return -1;
  }
  int len = strlen(buf);
  int res = fwrite(buf, 1, len, fd);
  if (res != len) {
    LOGE("[write]", "fwrite failed: %d <> %d ", res, len);
    res = fclose(fd);
    if (res) {
      LOGE("[write]", "fclose failed: %d", res);
      return -2;
    }
    return -3;
  }
  res = fclose(fd);
  if (res) {
    LOGE("[write]", "fclose failed: %d", res);
    return -4;
  }
  return 0;
}

//------------------------------
static int readFile(char *fname) {
  uint8_t buf[16];
  FILE *fd = fopen(fname, "rb");
  if (fd == NULL) {
    LOGE("[read]", "fopen failed");
    return -1;
  }
  int res = fread(buf, 1, 8, fd);
  if (res <= 0) {
    LOGE("[read]", "fread failed: %d", res);
    res = fclose(fd);
    if (res) {
      LOGE("[read]", "fclose failed: %d", res);
      return -2;
    }
    return -3;
  }
  res = fclose(fd);
  if (res) {
    LOGE("[read]", "fclose failed: %d", res);
    return -4;
  }
  return 0;
}

#define TEST_FILE_NUM (1000)
static int test_finished = 0;
/*
//================================
static void File_task_1(void *arg) {
  int res = 0;
  int n = 0;

  LOGI("[TASK_1]", "Started.");
  res = writeFile("/spiffs/testfil1.txt", "wb", "3");
  if (res == 0) {
    while (n < TEST_FILE_NUM) {
      n++;
      res = readFile("/spiffs/testfil1.txt");
      if (res != 0) {
        LOGE("[TASK_1]", "Error reading from file (%d), pass %d", res, n);
        break;
      }
      res = writeFile("/spiffs/testfil1.txt", "a", "3");
      if (res != 0) {
        LOGE("[TASK_1]", "Error writing to file (%d), pass %d", res, n);
        break;
      }
      vTaskDelay(2);
      if ((n % 100) == 0) {
        LOGI("[TASK_1]", "%d reads/writes", n);
      }
    }
    if (n == TEST_FILE_NUM) {
      LOGI("[TASK_1]", "Finished");
    }
  } else {
    LOGE("[TASK_1]", "Error creating file (%d)", res);
  }

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

//================================
static void File_task_2(void *arg) {
  int res = 0;
  int n = 0;

  LOGI("[TASK_2]", "Started.");
  res = writeFile("/spiffs/testfil2.txt", "wb", "3");
  if (res == 0) {
    while (n < TEST_FILE_NUM) {
      n++;
      res = readFile("/spiffs/testfil2.txt");
      if (res != 0) {
        LOGE("[TASK_2]", "Error reading from file (%d), pass %d", res, n);
        break;
      }
      res = writeFile("/spiffs/testfil2.txt", "a", "3");
      if (res != 0) {
        LOGE("[TASK_2]", "Error writing to file (%d), pass %d", res, n);
        break;
      }
      vTaskDelay(2);
      if ((n % 100) == 0) {
        LOGI("[TASK_2]", "%d reads/writes", n);
      }
    }
    if (n == TEST_FILE_NUM) {
      LOGI("[TASK_2]", "Finished");
    }
  } else {
    LOGE("[TASK_2]", "Error creating file (%d)", res);
  }

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

//================================
static void File_task_3(void *arg) {
  char tag[16] = { '\0' };
  int is_task = 1;
  if (arg) {
    is_task = 0;
    sprintf(tag, "Test");
  } else
    sprintf(tag, "[TASK_3]");
  int res = 0;
  uint32_t n = 0, min_w = 999999, max_w = 0, min_r = 999999, max_r = 0;
  uint64_t tstart, tend, trun, t_start;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  t_start = tv.tv_sec * 1000000L + tv.tv_usec;

  LOGI(tag, "Started.");
  res = writeFile("/spiffs/testfil3.txt", "wb", "3");
  if (res == 0) {
    while (n < TEST_FILE_NUM) {
      n++;
      gettimeofday(&tv, NULL);
      tstart = tv.tv_sec * 1000000L + tv.tv_usec;
      res = readFile("/spiffs/testfil3.txt");
      gettimeofday(&tv, NULL);
      tend = (tv.tv_sec * 1000000L + tv.tv_usec) - tstart;
      if (tend < min_r)
        min_r = tend;
      if (tend > max_r)
        max_r = tend;
      if (res != 0) {
        LOGE(tag, "Error reading from file (%d), pass %d", res, n);
        break;
      }
      gettimeofday(&tv, NULL);
      tstart = tv.tv_sec * 1000000L + tv.tv_usec;
      res = writeFile("/spiffs/testfil3.txt", "a", "3");
      gettimeofday(&tv, NULL);
      tend = (tv.tv_sec * 1000000L + tv.tv_usec) - tstart;
      if (tend < min_w)
        min_w = tend;
      if (tend > max_w)
        max_w = tend;
      if (res != 0) {
        LOGE(tag, "Error writing to file (%d), pass %d", res, n);
        break;
      }
      vTaskDelay(2);
      if ((n % 100) == 0) {
        LOGI(tag, "%d reads/writes", n);
      }
    }
    if (n == TEST_FILE_NUM) {
      gettimeofday(&tv, NULL);
      trun = (tv.tv_sec * 1000000L + tv.tv_usec) - t_start;
      LOGW(tag, "Min write time: %u us", min_w);
      LOGW(tag, "Max write time: %u us", max_w);
      LOGW(tag, "Min read  time: %u us", min_r);
      LOGW(tag, "Max read  time: %u us", max_r);
      LOGW(tag, "Total run time: %llu ms", trun / 1000);
      LOGI(tag, "Finished");
    }
  } else {
    LOGE(tag, "Error creating file (%d)", res);
  }

  vTaskDelay(1000 / portTICK_PERIOD_MS);
  printf("\r\n");
  list("/spiffs/", NULL);
  test_finished = 1;
  if (is_task) {
    while (1) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}
*/
int get_file_list_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: root path  <ex:/spiffs/>\n");
    return -1;
  }
  file_list(argv[1]);
  return 0;
}

int get_file_read_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: file absolute path <ex:/spiffs/test.txt>\n");
    return -1;
  }
  readTest(argv[1]);
  return 0;
}

// void get_file_list_cmd(int argc, char** argv) {}

//================
int test_usb_copy_file() {
  printf("\r\n\n");
  LOGI(tag, "==== STARTING SPIFFS TEST ====\n");

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // Remove test files
  remove("/spiffs/testfil1.txt");
  remove("/spiffs/testfil2.txt");
  remove("/spiffs/testfil3.txt");

  writeTest("/spiffs/test.txt");

  printf("========================\n");
  printf("==== Read from file ====\n");
  printf("========================\n");
  readTest("/spiffs/test.txt");

  printf("=================================================\n");
  printf("==== Read from file included in sfiffs image ====\n");
  printf("=================================================\n");
  readTest("/spiffs/spiffs.info");

  printf("=============================\n");
  printf("==== List root directory ====\n");
  printf("=============================\n");
  list("/spiffs/", NULL);

  mkdirTest("/spiffs/newdir");

  printf("================================================\n");
  printf("==== List content of the directory \"images\" ====\n");
  printf("==== which is included on spiffs image      ====\n");
  printf("================================================\n");
  list("/spiffs/images", NULL);
  /*
    printf("==============================================================\n");
    printf("==== Get the timings of spiffs operations\n");
    printf("==== Operation:\n");
    printf("====   Open file for writting, append 1 byte, close file\n");
    printf("====   Open file for readinging, read 8 bytes, close file\n");
    printf("==== 2 ms sleep between operations\n");
    printf("==== %4d operations will be executed\n", TEST_FILE_NUM);
    printf("==============================================================\n");
    int dummy = 0;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    File_task_3(&dummy);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    #ifdef 0
      printf("\n====================================================\n");
      printf("STARTING MULTITASK TEST (3 tasks created)\n");
      printf("Operation:\n");
      printf("  Open file for writting, append 1 byte, close file\n");
      printf("  Open file for readinging, read 8 bytes, close file\n");
      printf("2 ms sleep between operations\n");
      printf("Each task will perform %4d operations\n", TEST_FILE_NUM);
      printf("Expected run time 40~100 seconds\n");
      printf("====================================================\r\n");

      test_finished = 0;
      struct timeval tv;
      gettimeofday(&tv, NULL);
      uint32_t start_test_time = tv.tv_sec;
      xTaskCreatePinnedToCore(File_task_1, "FileTask1", 6 * 1024, NULL, 5, NULL, 1);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      xTaskCreatePinnedToCore(File_task_2, "FileTask2", 6 * 1024, NULL, 5, NULL, 1);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      xTaskCreatePinnedToCore(File_task_3, "FileTask3", 6 * 1024, NULL, 5, NULL, 1);
      while (!test_finished) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
      }
      gettimeofday(&tv, NULL);
      start_test_time = tv.tv_sec - start_test_time;
      vTaskDelay(500 / portTICK_PERIOD_MS);
      printf("\nTotal multitask test run time: %u seconds\n", start_test_time);
    #endif
    */
  printf("\n");
  LOGI(tag, "==== SPIFFS TEST FINISHED ====\n");
  return 0;
}
