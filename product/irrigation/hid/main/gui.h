#ifndef _GUI_H_
#define _GUI_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int lv_display_init(void);

bool lvgl_acquire(void);
void lvgl_release(void);

#ifdef __cplusplus
}
#endif

#endif  // _GUI_H_
