#ifndef _TH01_H_
#define _TH01_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  TH01_OK = 0,
  TH01_ERR_UART = -1,
  TH01_ERR_CHECKSUM = -2,
  TH01_ERR_STATUS = -3,
  TH01_ERR_PARAMS = -4,
} th01_err_code;

typedef struct {
  float temp;
  float mos;
} th01_data_t;

int th01_init(void);

int th01_read_manual(th01_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* _TH01_H_ */
