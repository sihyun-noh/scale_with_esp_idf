#ifndef _WIFI_JUDGE_H_
#define _WIFI_JUDGE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int data;
} wifi_ctx_t;

void wifi_init(void);
void wifi_status(void);
void wifi_connected(void);
void wifi_disconnected(void);

#ifdef __cplusplus
}
#endif
#endif /*_WIFI_JUDGE_H_ */
