#include "dajilog_ui_msg.h"

#include <string.h>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"

static portMUX_TYPE s_sequence_lock = portMUX_INITIALIZER_UNLOCKED;
static uint32_t s_next_sequence = 1;

void dajilog_ui_msg_init(dajilog_ui_msg_t *message,
                         dajilog_ui_msg_kind_t kind,
                         uint32_t topic)
{
    if (message == NULL) {
        return;
    }

    memset(message, 0, sizeof(*message));
    message->kind = kind;
    message->topic = topic;
    message->sequence = dajilog_ui_msg_next_sequence();
    message->timestamp_us = esp_timer_get_time();
    message->result = DAJILOG_UI_RESULT_OK;
}

uint32_t dajilog_ui_msg_next_sequence(void)
{
    uint32_t sequence;

    portENTER_CRITICAL(&s_sequence_lock);
    sequence = s_next_sequence++;
    if (s_next_sequence == 0) {
        s_next_sequence = 1;
    }
    portEXIT_CRITICAL(&s_sequence_lock);

    return sequence;
}
