#ifndef _SYSCFG_CMD_H_
#define _SYSCFG_CMD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Command to set a value of the system configuration key.
 *
 * @param argc  the number of arguments
 * @param argv  the arguments
 * @return int  0 on success, -1 on error
 */
int syscfg_set_cmd(int argc, char **argv);

/**
 * @brief Command to get a value of the system configuration key.
 *
 * @param argc  the number of arguments
 * @param argv  the arguments
 * @return int  0 on success, -1 on error
 */
int syscfg_get_cmd(int argc, char **argv);

/**
 * @brief Command to unset a value of the system configuration key.
 *
 * @param argc  the number of arguments
 * @param argv  the arguments
 * @return int  0 on success, -1 on error
 */
int syscfg_unset_cmd(int argc, char **argv);

/**
 * @brief Command to set the blob data of the system configuration key.
 *
 * @param argc the number of arguments
 * @param argv the arguments
 * @return int 0 on success, -1 on error
 */
int syscfg_set_blob_cmd(int argc, char **argv);

/**
 * @brief Command to get the blob data size of the system configuration key.
 *
 * @param argc the number of arguments
 * @param argv the a
 * @return int 0 on success, -1 on error
 */
int syscfg_get_blob_cmd(int argc, char **argv);

/**
 * @brief Command to erase all keys and values of the system configuration.
 *
 * @param argc  the number of arguments
 * @param argv  the arguments
 * @return int  0 on success, -1 on error
 */
int syscfg_erase_cmd(int argc, char **argv);

/**
 * @brief Command to show all keys and values of the system configuration.
 *
 * @param argc the number of arguments
 * @param argv the arguments
 * @return int 0 on success, -1 on error
 */
int syscfg_show_cmd(int argc, char **argv);

/**
 * @brief Command to show information(used and free entries) of the system configuration.
 *
 * @param argc the number of arguments
 * @param argv the arguments
 * @return int 0 on success, -1 on error
 */
int syscfg_info_cmd(int argc, char **argv);

/**
 * @breif Command to show all syscfg variables with CFG and MFG areas
 */
int syscfg_dump_cmd(int argc, char **argv);

/**
 * @breif Command to erase wifi and other information of nvs flash area
 */
int nvs_erase_cmd(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif /* _SYSCFG_CMD_H_ */
