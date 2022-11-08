#ifndef _SYSCFG_H_
#define _SYSCFG_H_

#include "esp32/syscfg_impl.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSCFG_VARIABLE_NAME_SIZE 32
#define SYSCFG_VARIABLE_VALUE_SIZE 64

/**
 * @brief Initialize the system configuration module.
 *
 * @return int 0 on success, -1 on error
 */
int syscfg_init(void);

/**
 * @brief Open the system configuration module.
 *
 * @return int 0 on success, -1 on error
 */
int syscfg_open(void);

/**
 * @brief Close the system configuration module.
 *
 * @return int 0 on success, -1 on error
 */
int syscfg_close(void);

/**
 * @brief Set the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key   the key to set
 * @param value the value to set
 * @return int  0 on success, -1 on error
 */
int syscfg_set(syscfg_type_t type, const char *key, const char *value);

/**
 * @brief Get the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key   the key to get
 * @param value the value to get
 * @param value_len the length of the value buffer
 * @return int  0 on success, -1 on error
 */
int syscfg_get(syscfg_type_t type, const char *key, char *value, size_t value_len);

/**
 * @brief Unset the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key  the key to unset
 * @return int 0 on success, -1 on error
 */
int syscfg_unset(syscfg_type_t type, const char *key);

/**
 * @brief Set the blob data of a system configuration key.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to set
 * @param value the value to set
 * @param value_len the length of the value buffer
 * @return int 0 on success, -1 on error
 */
int syscfg_set_blob(syscfg_type_t type, const char *key, const void *value, size_t value_len);

/**
 * @brief Get the blob data size of a system configuration key.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to get
 * @return int blob data size on success, -1 on error
 *         The blob data size is 0 if the key does not exist.
 */
int syscfg_get_blob_size(syscfg_type_t type, const char *key);

/**
 * @brief Get the blob data of a system configuration key.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to get
 * @param value the value to get
 * @param value_len the value buffer size
 * @return int 0 on success, -1 on error
 */
int syscfg_get_blob(syscfg_type_t type, const char *key, void *value, size_t value_len);

/**
 * @brief Clear all keys and values of system configuration module.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_clear(syscfg_type_t type);

/**
 * @brief Show all keys and values of system configuration module.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_show(syscfg_type_t type);

/**
 * @brief Show information(used and free entries) of system configuration module.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_info(syscfg_type_t type);

/**
 * @brief Erase all information of nvs flash area.
 *
 * @return int 0 on success, -1 on error
 */
int nvs_erase(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSCFG_H_ */
