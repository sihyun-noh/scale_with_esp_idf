#ifndef _EASY_SETUP_H_
#define _EASY_SETUP_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_ETHERNET_PACKAGE)
/**
 * @brief Initialize ethernet easy setup
 *
 */
void create_ethernet_easy_setup_task(void);

#else

/**
 * @brief Initialize easy setup
 *
 */
void create_easy_setup_task(void);

/**
 * @brief Check if setup taks is running
 *
 * @return bool true if taks is running, false if task is done
 */
bool is_running_setup_task(void);
#endif  // CONFIG_ETHERNET_PACKAGE

#ifdef __cplusplus
}
#endif

#endif /* _EASY_SETUP_H_ */
