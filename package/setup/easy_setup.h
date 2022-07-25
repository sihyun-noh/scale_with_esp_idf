#ifndef _EASY_SETUP_H_
#define _EASY_SETUP_H_

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
 * @brief Check connection with router
 *
 * @return int 0 on success, -1 on failure
 */
int is_router_connect(void);

#endif /* _EASY_SETUP_H_ */
