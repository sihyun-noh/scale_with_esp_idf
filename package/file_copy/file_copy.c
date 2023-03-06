#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>

#include "esp_vfs.h"
#include "file_copy.h"

#define FNM_NOMATCH 1         // Match failed.
#define FNM_NOESCAPE 0x01     // Disable backslash escaping.
#define FNM_PATHNAME 0x02     // Slash must be matched by slash.
#define FNM_PERIOD 0x04       // Period must be matched by period.
#define FNM_LEADING_DIR 0x08  // Ignore /<tail> after Imatch.
#define FNM_CASEFOLD 0x10     // Case insensitive search.
#define FNM_PREFIX_DIRS 0x20  // Directory prefixes of pattern match too.
#define EOS '\0'

#define MAX_BUF (1024 * 50)

static const char *rangematch(const char *pattern, char test, int flags) {
  int negate, ok;
  char c, c2;

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

  dir = opendir(path);
  if (!dir) {
    printf("Error opening directory\n");
    return;
  }

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

  printf("-----------------------------------\n\n");
}

void file_list(const char *path) {
  list((char *)path, NULL);
}

int file_copy(const char *to, const char *from) {
  FILE *fd_to;
  FILE *fd_from;
  char buf[1024];
  ssize_t nread;
  int saved_errno;

  fd_from = fopen(from, "rb");
  if (fd_from == NULL)
    return -1;

  fd_to = fopen(to, "wb");
  if (fd_to == NULL)
    goto out_error;

  while ((nread = fread(buf, 1, sizeof(buf), fd_from)) > 0) {
      if (!fwrite(buf, 1, nread, fd_to))
        goto out_error;
  }

  printf("nread1 = %d\n", nread);
  if (nread == 0) {
    if (fclose(fd_to) < 0) {
      fd_to = NULL;
      goto out_error;
    }
    fclose(fd_from);

    printf("file copy success!!! from : %s to %s\n", from, to);

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

void readTest(char *fname) {
  printf("  file: \"%s\"\n", fname);

  int res;
  char *buf;
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

int get_file_list_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: root path  <ex:/storage/>\n");
    return -1;
  }
  file_list(argv[1]);
  return 0;
}

int get_file_read_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: file absolute path <ex:/storage/test.txt>\n");
    return -1;
  }
  readTest(argv[1]);
  return 0;
}

int make_dir(const char *path) {
  if (mkdir(path, 0777) == -1 && errno != EEXIST) {
    printf("Create dir [%s] failed : %s", path, strerror(errno));
    return -1;
  }
  return 0;
}
