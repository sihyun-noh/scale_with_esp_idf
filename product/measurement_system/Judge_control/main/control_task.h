#ifndef _CONTROL_TASK_H_
#define _CONTROL_TASK_H_

#ifdef __cplusplus
extern "C" {
#endif

void send_msg_to_ctrl_task(void *msg, size_t msg_len);
void create_control_task(char *indicator);

/**
 * @brief Create a pattern interrupt task
 *
 * @return int
 */

int uart_interrupt_config(void);

#ifdef __cplusplus
}
#endif

#endif  // _CONTROL_TASK_H_
