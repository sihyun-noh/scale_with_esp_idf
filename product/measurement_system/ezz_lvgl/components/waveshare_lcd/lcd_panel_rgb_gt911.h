#ifndef _LCD_RGB_GT911_
#define _LCD_RGB_GT911_

#ifdef __cplusplus
extern "C" {
#endif

extern void lv_port_disp_init(void);
extern void lv_port_indev_init(void);
extern esp_err_t lv_port_tick_init(void);
extern void lv_port_12c_tp_init();
extern lv_disp_t *disp;
#ifdef __cplusplus
}
#endif

#endif
