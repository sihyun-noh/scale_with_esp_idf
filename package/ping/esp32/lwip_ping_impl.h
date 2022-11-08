#ifndef _LWIP_PING_IMPL_H_
#define _LWIP_PING_IMPL_H_

#include "const.h"

#ifdef __cplusplus
extern "C" {
#endif

int do_ping_lwip_impl(char *host, int seq);

#ifdef __cplusplus
}
#endif

#endif /* _LWIP_PING_IMPL_H_ */
