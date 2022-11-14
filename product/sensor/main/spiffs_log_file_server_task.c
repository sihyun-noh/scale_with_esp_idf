#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp32/spiffs_impl.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_http_server.h"

#include "syslog.h"
#include "filelog.h"
#include "sysevent.h"
#include "event_ids.h"
#include "monitoring.h"
#include "sysfile.h"

static const char *TAG = "spiffs_file_server";

static TaskHandle_t log_file_server_task_handle = NULL;

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Scratch buffer size */
#define SCRATCH_BUFSIZE 8192

struct file_server_data {
  char base_path[ESP_VFS_PATH_MAX + 1];
  char scratch[SCRATCH_BUFSIZE];
};

/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories */
static esp_err_t http_resp_dir_html(httpd_req_t *req, const char *dirpath) {
  char entrypath[FILE_PATH_MAX];
  char entrysize[16];
  const char *entrytype;

  struct dirent *entry;
  struct stat entry_stat;

  DIR *dir = opendir(dirpath);
  const size_t dirpath_len = strlen(dirpath);

  /* Retrieve the base path of file storage to construct the full path */
  strlcpy(entrypath, dirpath, sizeof(entrypath));

  if (!dir) {
    LOGE(TAG, "Failed to stat dir : %s", dirpath);
    /* Respond with 404 Not Found */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
    return ESP_FAIL;
  }

  /* Send HTML file header */
  httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");

  /* Send file-list table definition and column labels */
  httpd_resp_sendstr_chunk(
      req,
      "<table class=\"fixed\" border=\"1\">"
      "<col width=\"800px\" /><col width=\"300px\" /><col width=\"300px\" /><col width=\"100px\" />"
      "<thead><tr><th>Name</th><th>Type</th><th>Size (Bytes)</th><th>Download</th></tr></thead>"
      "<tbody>");

  /* Iterate over all files / folders and fetch their names and sizes */
  while ((entry = readdir(dir)) != NULL) {
    entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

    strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
    if (stat(entrypath, &entry_stat) == -1) {
      LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
      continue;
    }
    sprintf(entrysize, "%ld", entry_stat.st_size);
    LOGI(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize);

    /* Send chunk of HTML file containing table entries with file name and size */
    httpd_resp_sendstr_chunk(req, "<tr><td><a href=\"");
    httpd_resp_sendstr_chunk(req, req->uri);
    httpd_resp_sendstr_chunk(req, entry->d_name);
    if (entry->d_type == DT_DIR) {
      httpd_resp_sendstr_chunk(req, "/");
    }
    httpd_resp_sendstr_chunk(req, "\">");
    httpd_resp_sendstr_chunk(req, entry->d_name);
    httpd_resp_sendstr_chunk(req, "</a></td><td>");
    httpd_resp_sendstr_chunk(req, entrytype);
    httpd_resp_sendstr_chunk(req, "</td><td>");
    httpd_resp_sendstr_chunk(req, entrysize);
    httpd_resp_sendstr_chunk(req, "</td><td>");
    httpd_resp_sendstr_chunk(req, "<a href=\"");
    httpd_resp_sendstr_chunk(req, req->uri);
    httpd_resp_sendstr_chunk(req, entry->d_name);
    if (entry->d_type == DT_DIR) {
      httpd_resp_sendstr_chunk(req, "/");
    }
    httpd_resp_sendstr_chunk(req, "\" download>");
    httpd_resp_sendstr_chunk(req, "download");
    httpd_resp_sendstr_chunk(req, "</a></td><td>");

    httpd_resp_sendstr_chunk(req, "</td></tr>\n");
  }
  closedir(dir);

  /* Finish the file list table */
  httpd_resp_sendstr_chunk(req, "</tbody></table>");

  /* Send remaining chunk of HTML file to complete it */
  httpd_resp_sendstr_chunk(req, "</body></html>");

  /* Send empty chunk to signal HTTP response completion */
  httpd_resp_sendstr_chunk(req, NULL);
  return ESP_OK;
}

#define IS_FILE_EXT(filename, ext) (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename) {
  if (IS_FILE_EXT(filename, ".pdf")) {
    return httpd_resp_set_type(req, "application/pdf");
  } else if (IS_FILE_EXT(filename, ".html")) {
    return httpd_resp_set_type(req, "text/html");
  } else if (IS_FILE_EXT(filename, ".jpeg")) {
    return httpd_resp_set_type(req, "image/jpeg");
  } else if (IS_FILE_EXT(filename, ".ico")) {
    return httpd_resp_set_type(req, "image/x-icon");
  }
  /* This is a limited set only */
  /* For any other type always set as plain text */
  return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char *get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize) {
  const size_t base_pathlen = strlen(base_path);
  size_t pathlen = strlen(uri);

  const char *quest = strchr(uri, '?');
  if (quest) {
    pathlen = MIN(pathlen, quest - uri);
  }
  const char *hash = strchr(uri, '#');
  if (hash) {
    pathlen = MIN(pathlen, hash - uri);
  }

  if (base_pathlen + pathlen + 1 > destsize) {
    /* Full path string won't fit into destination buffer */
    return NULL;
  }

  /* Construct full path (base + path) */
  strcpy(dest, base_path);
  strlcpy(dest + base_pathlen, uri, pathlen + 1);

  /* Return pointer to path, skipping the base */
  return dest + base_pathlen;
}

/* Handler to download a file kept on the server */
static esp_err_t download_get_handler(httpd_req_t *req) {
  char filepath[FILE_PATH_MAX];
  FILE *fd = NULL;
  struct stat file_stat;

  const char *filename =
      get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path, req->uri, sizeof(filepath));
  if (!filename) {
    LOGE(TAG, "Filename is too long");
    /* Respond with 500 Internal Server Error */
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
    return ESP_FAIL;
  }

  /* If name has trailing '/', respond with directory contents */
  if (filename[strlen(filename) - 1] == '/') {
    return http_resp_dir_html(req, filepath);
  }

  if (stat(filepath, &file_stat) == -1) {
    LOGE(TAG, "Failed to stat file : %s", filepath);
    /* Respond with 404 Not Found */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
    return ESP_FAIL;
  }

  fd = fopen(filepath, "r");
  if (!fd) {
    LOGE(TAG, "Failed to read existing file : %s", filepath);
    /* Respond with 500 Internal Server Error */
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
    return ESP_FAIL;
  }

  LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
  set_content_type_from_file(req, filename);

  /* Retrieve the pointer to scratch buffer for temporary storage */
  char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
  size_t chunksize;
  do {
    /* Read file in chunks into the scratch buffer */
    chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);
    if (chunksize > 0) {
      /* Send the buffer contents as HTTP response chunk */
      if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
        fclose(fd);
        LOGE(TAG, "File sending failed!");
        /* Abort sending file */
        httpd_resp_sendstr_chunk(req, NULL);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
        return ESP_FAIL;
      }
    }

    /* Keep looping till the whole file is sent */
  } while (chunksize != 0);

  /* Close file after sending complete */
  fclose(fd);
  LOGI(TAG, "File sending complete");

  /* Respond with an empty chunk to signal HTTP response completion */
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
  httpd_resp_set_hdr(req, "Connection", "close");
#endif
  httpd_resp_send_chunk(req, NULL, 0);
  return ESP_OK;
}

/* Function to start the file server */
esp_err_t start_file_server_impl(const char *base_path, uint32_t port) {
  static struct file_server_data *server_data = NULL;

  /* Validate file storage base path */
  if (!base_path || strcmp(base_path, "/spiffs") != 0) {
    LOGE(TAG, "File server presently supports only '/spiffs' as base path");
    return ESP_ERR_INVALID_ARG;
  }

  if (server_data) {
    LOGE(TAG, "File server already started");
    return ESP_ERR_INVALID_STATE;
  }

  /* Allocate memory for server data */
  server_data = calloc(1, sizeof(struct file_server_data));
  if (!server_data) {
    LOGE(TAG, "Failed to allocate memory for server data");
    return ESP_ERR_NO_MEM;
  }
  strlcpy(server_data->base_path, base_path, sizeof(server_data->base_path));

  httpd_handle_t file_log_server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  /* Use the URI wildcard matching function in order to
   * allow the same handler to respond to multiple different
   * target URIs which match the wildcard scheme */
  config.uri_match_fn = httpd_uri_match_wildcard;
  config.server_port = port;

  LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
  if (httpd_start(&file_log_server, &config) != ESP_OK) {
    LOGE(TAG, "Failed to start file server!");
    return ESP_FAIL;
  }

  /* URI handler for getting uploaded files */
  httpd_uri_t file_stream = {
    .uri = "/*",  // Match all URIs of type /path/to/file
    .method = HTTP_GET,
    .handler = download_get_handler,
    .user_ctx = server_data  // Pass server data as context
  };
  httpd_register_uri_handler(file_log_server, &file_stream);

  return ESP_OK;
}

int start_file_server(uint32_t port) {
  return start_file_server_impl(BASE_PATH, port);
}

static void log_file_server_task(void *pvParameters) {
  /*  log file server start! */
  start_file_server(8000);
  while (1) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

int create_log_file_server_task(void) {
  UBaseType_t task_priority = tskIDLE_PRIORITY + 5;

  if (!log_file_server_task_handle) {
    xTaskCreate(log_file_server_task, "log_file_task", 4096, NULL, task_priority, &log_file_server_task_handle);
    if (!log_file_server_task_handle) {
      LOGI(TAG, "log_file_server_task Task Start Failed!");
      return -1;
    }
  }
  return 0;
}
