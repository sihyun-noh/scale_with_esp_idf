#ifndef _MSC_H_
#define _MSC_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize usb host msc
 *
 */
void create_usb_host_msc_task(void);

#ifdef __cplusplus
}
#endif

#endif /* _MSC_H_ */
