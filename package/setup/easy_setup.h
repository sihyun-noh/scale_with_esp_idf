#ifndef _EASY_SETUP_H_
#define _EASY_SETUP_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize easy setup
 *
 */
void create_easy_setup_task(void);

/**
 * @brief Initialize ethernet easy setup
 *
 */
void create_ethernet_easy_setup_task(void);

/**
 * @brief Check if setup taks is running
 *
 * @return bool true if taks is running, false if task is done
 */
bool is_running_setup_task(void);

#ifdef __cplusplus
}
#endif

#endif /* _EASY_SETUP_H_ */
