#ifndef _OTA_TASK_H_
#define _OTA_TASK_H_

void start_ota_fw_task(char* fw_download_url);
int start_ota_fw_task_wait(char* fw_download_url);

#endif /* _OTA_TASK_H_ */
