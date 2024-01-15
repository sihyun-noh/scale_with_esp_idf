#ifndef _I2S_SPEAKER__
#define _I2S_SPEAKER__

#ifdef __cplusplus
    extern "C"{
#endif
/**
 * @brief i2s 
 * 
 * @return int 
 */
int i2s_speak_init(void);

void dingdong_volume(void);
void normal_volume(void);
void over_volume(void);
void lack_volume(void);

#ifdef __cplusplus
}
#endif


#endif
