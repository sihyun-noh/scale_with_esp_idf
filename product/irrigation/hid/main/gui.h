#ifndef _GUI_H_
#define _GUI_H_

#ifdef __cplusplus
extern "C" {
#endif

int lv_display_init(void);

void lvgl_acquire(void);
void lvgl_release(void);

#ifdef __cplusplus
}
#endif

#endif  // _GUI_H_
