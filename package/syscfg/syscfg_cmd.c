/**
 * @file syscfg_cmd.c
 *
 * @brief Command to manipulate the system configuration key and value.
 *
 * Created by Greenlabs, Smartfarm Team.
 * Copyright (c) 2022 Greenlabs Co. and/or its affiliates. All rights reserved.

 * To obtain a license, please contact Greenlabs.

 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. GREENLABS SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 *
 */
#include "syscfg.h"
#include "sys_config.h"

#include <string.h>

int syscfg_get_cmd(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <type:0,1> <key>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);
  char *key = argv[2];
  char value[SYSCFG_VARIABLE_VALUE_SIZE] = { 0 };
  if (syscfg_get(type, key, value, sizeof(value)) != 0) {
    printf("Failed to get value for key %s\n", key);
    return -1;
  }

  printf("name = %s, value = %s\n", key, value);
  return 0;
}

int syscfg_set_cmd(int argc, char **argv) {
  if (argc != 4) {
    printf("Usage: %s <type:0,1> <key> <value>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);
  char *key = argv[2];
  char *value = argv[3];

  if (syscfg_set(type, key, value) != 0) {
    printf("Failed to set value for key %s\n", key);
    return -1;
  }

  return 0;
}

int syscfg_unset_cmd(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <type:0,1> <key>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);
  char *key = argv[2];

  if (syscfg_unset(type, key) != 0) {
    printf("Failed to unset value for key %s\n", key);
    return -1;
  }

  return 0;
}

int syscfg_set_blob_cmd(int argc, char **argv) {
  if (argc != 4) {
    printf("Usage: %s <type:0,1> <key> <value>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);
  char *key = argv[2];
  char *value = argv[3];

  if (syscfg_set_blob(type, key, value, strlen(value)) != 0) {
    printf("Failed to set value for key %s\n", key);
    return -1;
  }

  return 0;
}

int syscfg_get_blob_cmd(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <type:0,1> <key>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);
  char *key = argv[2];
  int blob_len = syscfg_get_blob_size(type, key);
  char *blob_data = (char *)malloc(blob_len + 1);

  if (syscfg_get_blob(type, key, blob_data, blob_len) == 0) {
    /* blob_len inclues length of blob data with zero terminate */
    blob_data[blob_len] = '\0';
    printf("name = %s, value = %s\n", key, blob_data);
    if (blob_data) {
      free(blob_data);
    }
  } else {
    printf("Failed to get value for key %s\n", key);
  }
  return 0;
}

int syscfg_erase_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (syscfg_clear(type) != 0) {
    printf("Failed to clear type %d\n", type);
    return -1;
  }

  return 0;
}

int syscfg_show_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (syscfg_show(type) != 0) {
    printf("Failed to get all key and value from the parition = %d\n", type);
    return -1;
  }

  return 0;
}

int syscfg_info_cmd(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s <type:0,1>\n", argv[0]);
    return -1;
  }

  int type = atoi(argv[1]);

  if (syscfg_info(type) != 0) {
    printf("Failed to get used and free info from the partition = %d\n", type);
    return -1;
  }

  return 0;
}

int syscfg_dump_cmd(int argc, char **argv) {
  dump_syscfg();
  return 0;
}

int nvs_erase_cmd(int argc, char **argv) {
  nvs_erase();
  return 0;
}
