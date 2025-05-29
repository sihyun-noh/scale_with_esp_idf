
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_console.h"
#include "esp_log.h"
#include "argtable3/argtable3.h"
#include "nvs.h"
#include "nvs_ext.h"  // nvs_write_str, nvs_read_str 등 nvs_ext 관련 함수들
#include "nvs_cfg.h"
#include "sensor_cfg_config.h"
#include "console_init.h"
#include "sdkconfig.h"

static const char *TAG = "console_nvs";

// -----------------------------------------------------------------------------
// get_nvs_list : NVS에 저장된 key 목록을 표시하는 명령어 (인자는 없음)
static int do_get_nvs_list_cmd(int argc, char **argv) {
  ESP_LOGI(TAG, "NVS Key List:");

  // "nvs_ext_data" 파티션 내 모든 엔트리를 찾습니다.
  nvs_iterator_t it = NULL;
  esp_err_t err = nvs_entry_find("nvs", NULL, NVS_TYPE_ANY, &it);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "No NVS entries found in partition 'nvs_ext_data'.");
    return 0;
  } else if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_entry_find failed: %s", esp_err_to_name(err));
    return 1;
  }

  // iterator를 사용하여 각 엔트리를 순회합니다.
  while (it != NULL) {
    nvs_entry_info_t info;
    err = nvs_entry_info(it, &info);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to get entry info: %s", esp_err_to_name(err));
      break;
    }
    ESP_LOGI(TAG, "Key: %s", info.key);
    err = nvs_entry_next(&it);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
      break;
    } else if (err != ESP_OK) {
      ESP_LOGE(TAG, "nvs_entry_next error: %s", esp_err_to_name(err));
      break;
    }
  }

  // 할당된 iterator를 해제합니다.
  if (it != NULL) {
    nvs_release_iterator(it);
  }
  return 0;
}

// -----------------------------------------------------------------------------
// set_nvs_data : 지정한 key에 데이터를 저장하는 명령어
// 예제에서는 문자열 데이터를 저장합니다.
static struct {
  struct arg_str *key;
  struct arg_str *value;
  struct arg_end *end;
} set_nvs_args;

static int do_set_nvs_data_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&set_nvs_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, set_nvs_args.end, argv[0]);
    return 1;
  }
  const char *key = set_nvs_args.key->sval[0];
  const char *value = set_nvs_args.value->sval[0];
  ESP_LOGI(TAG, "Setting NVS data: key='%s', value='%s'", key, value);

  esp_err_t err = nvs_write_str(key, value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_write_str failed: %s", esp_err_to_name(err));
    return 1;
  }
  return 0;
}

// -----------------------------------------------------------------------------
// get_nvs_data : 지정한 key에 해당하는 데이터를 읽어오는 명령어
static struct {
  struct arg_str *key;
  struct arg_end *end;
} get_nvs_args;

static int do_get_nvs_data_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_nvs_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_nvs_args.end, argv[0]);
    return 1;
  }
  const char *key = get_nvs_args.key->sval[0];
  char *read_value = NULL;
  esp_err_t err = nvs_read_str(key, &read_value);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_read_str failed: %s", esp_err_to_name(err));
    return 1;
  }
  ESP_LOGI(TAG, "Read NVS data: key='%s', value='%s'", key, read_value);
  free(read_value);
  return 0;
}

// -----------------------------------------------------------------------------
// cfg
// get_cfg_list : cfg 파티션에 저장된 key 목록을 표시
static int do_get_cfg_list_cmd(int argc, char **argv) {
  ESP_LOGI(TAG, "CFG Key List:");
  nvs_iterator_t it = NULL;
  esp_err_t err = nvs_entry_find("cfg", NULL, NVS_TYPE_ANY, &it);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "No entries found in cfg partition.");
    return 0;
  } else if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_entry_find failed: %s", esp_err_to_name(err));
    return 1;
  }
  while (it != NULL) {
    nvs_entry_info_t info;
    err = nvs_entry_info(it, &info);
    if (err != ESP_OK)
      break;
    ESP_LOGI(TAG, "Key: %s", info.key);
    err = nvs_entry_next(&it);
    if (err == ESP_ERR_NVS_NOT_FOUND)
      break;
    if (err != ESP_OK)
      break;
  }
  if (it)
    nvs_release_iterator(it);
  return 0;
}

// -----------------------------------------------------------------------------
static struct {
  struct arg_str *key;
  struct arg_str *value;
  struct arg_end *end;
} set_str_args;

static int do_set_cfg_str_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&set_str_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, set_str_args.end, argv[0]);
    return 1;
  }
  const char *key = set_str_args.key->sval[0];
  const char *value = set_str_args.value->sval[0];
  ESP_LOGI(TAG, "Setting cfg string: %s = %s", key, value);
  return cfg_set_str(key, value);
}

static struct {
  struct arg_str *key;
  struct arg_end *end;
} get_str_args;

static int do_get_cfg_str_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_str_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_str_args.end, argv[0]);
    return 1;
  }
  const char *key = get_str_args.key->sval[0];
  char *val = NULL;
  esp_err_t err = cfg_get_str(key, &val);
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Read: %s = %s", key, val);
    free(val);
  }
  return err;
}

static struct {
  struct arg_str *key;
  struct arg_end *end;
} set_blob_args, get_blob_args;

static int do_set_cfg_blob_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&set_blob_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, set_blob_args.end, argv[0]);
    return 1;
  }
  const char *key = set_blob_args.key->sval[0];
  typedef struct {
    int id;
    float value;
    char label[16];
  } blob_t;
  blob_t b = { .id = 7, .value = 42.42f };
  strcpy(b.label, "default");
  return cfg_set_blob(key, &b, sizeof(b));
}

static int do_get_cfg_blob_cmd(int argc, char **argv) {
  int nerrors = arg_parse(argc, argv, (void **)&get_blob_args);
  if (nerrors != 0) {
    arg_print_errors(stderr, get_blob_args.end, argv[0]);
    return 1;
  }
  const char *key = get_blob_args.key->sval[0];
  /* typedef struct { */
  /*   int id; */
  /*   float value; */
  /*   char label[16]; */
  /* } blob_t; */
  /* blob_t b; */
  sensor_port_cfg_t b;
  esp_err_t err = cfg_get_blob(key, &b, sizeof(b));
  if (err == ESP_OK) {
    ESP_LOGI(TAG, "Read blob: port=%d, dirty=%d, enabled=%d, sensor_type=%d, threshold=%d", b.port, b.dirty, b.enabled,
             b.sensor_type, b.threshold);
  }
  return err;
}

// cfg end
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// 콘솔 명령어 등록 함수
esp_err_t console_cmd_nvs_register(void) {
  esp_err_t ret = ESP_OK;

  // 1. get_nvs_list 명령어 등록 (인자 없음)
  const esp_console_cmd_t get_list_cmd = { .command = "get_nvs_list",
                                           .help = "Display the list of NVS keys",
                                           .hint = NULL,
                                           .func = &do_get_nvs_list_cmd,
                                           .argtable = NULL };
  ret = esp_console_cmd_register(&get_list_cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register get_nvs_list command");
    return ret;
  }

  // 2. set_nvs_data 명령어 등록 (필수 인자: key, value)
  set_nvs_args.key = arg_str1("k", "key", "<key>", "Key for NVS data");
  set_nvs_args.value = arg_str1("v", "value", "<value>", "Value to store");
  set_nvs_args.end = arg_end(2);
  const esp_console_cmd_t set_data_cmd = { .command = "set_nvs_data",
                                           .help = "Store data in NVS for the given key",
                                           .hint = NULL,
                                           .func = &do_set_nvs_data_cmd,
                                           .argtable = &set_nvs_args };
  ret = esp_console_cmd_register(&set_data_cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register set_nvs_data command");
    return ret;
  }

  // 3. get_nvs_data 명령어 등록 (필수 인자: key)
  get_nvs_args.key = arg_str1("k", "key", "<key>", "Key for NVS data");
  get_nvs_args.end = arg_end(1);
  const esp_console_cmd_t get_data_cmd = { .command = "get_nvs_data",
                                           .help = "Retrieve data from NVS for the given key",
                                           .hint = NULL,
                                           .func = &do_get_nvs_data_cmd,
                                           .argtable = &get_nvs_args };
  ret = esp_console_cmd_register(&get_data_cmd);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register get_nvs_data command");
    return ret;
  }

  // cfg
  const esp_console_cmd_t cmd_list = { .command = "get_cfg_list",
                                       .help = "List all keys in cfg partition",
                                       .hint = NULL,
                                       .func = &do_get_cfg_list_cmd,
                                       .argtable = NULL };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_list));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register get_cfg_list command");
    return ret;
  }

  set_str_args.key = arg_str1("k", "key", "<key>", "Key");
  set_str_args.value = arg_str1("v", "value", "<value>", "String value");
  set_str_args.end = arg_end(2);
  const esp_console_cmd_t cmd_set_str = { .command = "set_cfg_str",
                                          .help = "Set string value to cfg",
                                          .hint = NULL,
                                          .func = &do_set_cfg_str_cmd,
                                          .argtable = &set_str_args };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_set_str));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register set_cfg_str command");
    return ret;
  }

  get_str_args.key = arg_str1("k", "key", "<key>", "Key");
  get_str_args.end = arg_end(1);
  const esp_console_cmd_t cmd_get_str = { .command = "get_cfg_str",
                                          .help = "Get string value from cfg",
                                          .hint = NULL,
                                          .func = &do_get_cfg_str_cmd,
                                          .argtable = &get_str_args };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_get_str));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register get_cfg_str command");
    return ret;
  }

  set_blob_args.key = arg_str1("k", "key", "<key>", "Key");
  set_blob_args.end = arg_end(1);
  const esp_console_cmd_t cmd_set_blob = { .command = "set_cfg_blob",
                                           .help = "Set struct blob to cfg",
                                           .hint = NULL,
                                           .func = &do_set_cfg_blob_cmd,
                                           .argtable = &set_blob_args };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_set_blob));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register set_cfg_blob command");
    return ret;
  }

  get_blob_args.key = arg_str1("k", "key", "<key>", "Key");
  get_blob_args.end = arg_end(1);
  const esp_console_cmd_t cmd_get_blob = { .command = "get_cfg_blob",
                                           .help = "Get struct blob from cfg",
                                           .hint = NULL,
                                           .func = &do_get_cfg_blob_cmd,
                                           .argtable = &get_blob_args };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd_get_blob));
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register get_cfg_blob command");
    return ret;
  }

  return ret;
}

// -----------------------------------------------------------------------------
// CONFIG_NVS_CMD_AUTO_REGISTRATION 옵션이 활성화되어 있을 경우
// 플러그인 등록을 통한 자동 등록 기능 구현 (링커 파일의 설정에 따름)
#if CONFIG_NVS_CMD_AUTO_REGISTRATION
static const console_cmd_plugin_desc_t
    __attribute__((section(".console_cmd_desc"), used)) PLUGIN = { .name = "console_cmd_nvs",
                                                                   .plugin_regd_fn = &console_cmd_nvs_register };
#endif
