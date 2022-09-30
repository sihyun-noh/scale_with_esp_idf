#ifndef _SYSCFG_IMPL_H_
#define _SYSCFG_IMPL_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

/**
 * @brief The error type of the syscfg
 */
enum {
  SYSCFG_NO_ERR = 0,
  SYSCFG_ERR_INIT = -1,
  SYSCFG_ERR_OPEN = -2,
  SYSCFG_ERR_CLOSE = -3,
  SYSCFG_ERR_GET = -4,
  SYSCFG_ERR_SET = -5,
  SYSCFG_ERR_UNSET = -6,
  SYSCFG_ERR_COMMIT = -7,
  SYSCFG_ERR_ERASE = -8,
  SYSCFG_ERR_MUTEX = -9,
  SYSCFG_ERR_NVS_HANDLE = -10,
};

typedef enum syscfg_type {
  CFG_DATA, /*<! configuration data of syscfg partition */
  MFG_DATA, /*<! manufacturing data of manufacturing partition */
  NO_DATA,  /*<! data type to indicate end data */
} syscfg_type_t;

/**
 * @brief Initialize the system configuration module for esp32.
 *
 * @return 0 on success, -1 on error
 */
int syscfg_init_impl(void);

/**
 * @brief Open the system configuration module for esp32.
 *
 * @return 0 on success, -1 on error
 */
int syscfg_open_impl(void);

/**
 * @brief Close the system configuration module for esp32.
 *
 * @return 0 on success, -1 on error
 */
int syscfg_close_impl(void);

/**
 * @brief Set the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key   the key to set
 * @param value the value to set
 * @return int  0 on success, -1 on error
 */
int syscfg_set_impl(syscfg_type_t type, const char *key, const char *value);

/**
 * @brief Get the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key   the key to get
 * @param value the value to get
 * @param value_len the length of the value buffer
 * @return int 0 on success, -1 on error
 */
int syscfg_get_impl(syscfg_type_t type, const char *key, char *value, size_t value_len);

/**
 * @brief Unset the value of a system configuration key.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @param key  the key to unset
 * @return int 0 on success, -1 on error
 */
int syscfg_unset_impl(syscfg_type_t type, const char *key);

/**
 * @brief Clear all keys and values of system configuration module.
 *
 * @note  Manufacturer data(mfg_data) can not be cleared.
 *
 * @param type  the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_clear_impl(syscfg_type_t type);

/**
 * @brief Set blob data of a system configuration key.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to set
 * @param blob the blob data to set
 * @param blob_len the length of the blob data
 * @return int 0 on success, -1 on error
 */
int syscfg_set_blob_impl(syscfg_type_t type, const char *key, const void *blob, size_t blob_len);

/**
 * @brief Get blob data size of a system configuration key.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to get
 * @return int 0 or blob_len on success, -1 on error
 *         blob_len the actual length of the blob data that was located in flash area
 *         The blob_len will be used to allocate buffer before syscfg_get_blob_impl() to get the blob data.
 */
int syscfg_get_blob_size_impl(syscfg_type_t type, const char *key);

/**
 * @brief Get blob data of a system configuration key
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @param key the key to get
 * @param blob the blob data to get
 * @param blob_len the length of the blob data
 * @return int 0 on success, -1 on error
 */
int syscfg_get_blob_impl(syscfg_type_t type, const char *key, void *blob, size_t blob_len);

/**
 * @brief Show all keys and values of system configuration module.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_show_impl(syscfg_type_t type);

/**
 * @brief Show information (used and free entries) of system configuration module.
 *
 * @param type the type of system configuration (cfg_data or mfg_data)
 * @return int 0 on success, -1 on error
 */
int syscfg_info_impl(syscfg_type_t type);

/**
 * @brief Erase all information of nvs flash area
 *
 * @return int 0 on success, -1 on error
 */
int nvs_erase_impl(void);

#ifdef __cplusplus
}
#endif

#endif /* _SYSCFG_IMPL_H_ */
