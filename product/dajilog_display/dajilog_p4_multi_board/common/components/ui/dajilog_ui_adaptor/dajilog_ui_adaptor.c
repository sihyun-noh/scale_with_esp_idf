#include "dajilog_ui_adaptor.h"

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static void bridge_observer(const dajilog_ui_msg_t *message, void *user_context)
{
    dajilog_ui_adaptor_t *adaptor = (dajilog_ui_adaptor_t *)user_context;
    if (adaptor == NULL || !adaptor->attached || adaptor->queue_handle == NULL) {
        return;
    }

    QueueHandle_t queue = (QueueHandle_t)adaptor->queue_handle;
    (void)xQueueSend(queue, message, 0);
}

esp_err_t dajilog_ui_adaptor_attach(
    dajilog_ui_adaptor_t *adaptor,
    const dajilog_ui_adaptor_config_t *config)
{
    if (adaptor == NULL || config == NULL || config->render == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (adaptor->attached) {
        return ESP_ERR_INVALID_STATE;
    }

    size_t queue_length = config->queue_length;
    if (queue_length == 0) {
        queue_length = DAJILOG_UI_ADAPTOR_DEFAULT_QUEUE_LENGTH;
    }

    QueueHandle_t queue = xQueueCreate(queue_length, sizeof(dajilog_ui_msg_t));
    if (queue == NULL) {
        return ESP_ERR_NO_MEM;
    }

    memset(adaptor, 0, sizeof(*adaptor));
    adaptor->queue_handle = queue;
    adaptor->render = config->render;
    adaptor->render_context = config->render_context;
    adaptor->attached = true;

    esp_err_t error = dajilog_ui_bridge_subscribe(
        bridge_observer, adaptor, &adaptor->subscription);
    if (error != ESP_OK) {
        adaptor->attached = false;
        vQueueDelete(queue);
        memset(adaptor, 0, sizeof(*adaptor));
        return error;
    }
    return ESP_OK;
}

esp_err_t dajilog_ui_adaptor_detach(dajilog_ui_adaptor_t *adaptor)
{
    if (adaptor == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!adaptor->attached) {
        return ESP_OK;
    }

    adaptor->attached = false;
    esp_err_t error = dajilog_ui_bridge_unsubscribe(adaptor->subscription);
    if (adaptor->queue_handle != NULL) {
        vQueueDelete((QueueHandle_t)adaptor->queue_handle);
    }
    memset(adaptor, 0, sizeof(*adaptor));
    return error == ESP_ERR_NOT_FOUND ? ESP_OK : error;
}

size_t dajilog_ui_adaptor_process(dajilog_ui_adaptor_t *adaptor,
                                  size_t max_messages)
{
    if (adaptor == NULL || !adaptor->attached ||
        adaptor->queue_handle == NULL || adaptor->render == NULL) {
        return 0;
    }

    size_t processed = 0;
    dajilog_ui_msg_t message;
    QueueHandle_t queue = (QueueHandle_t)adaptor->queue_handle;
    while ((max_messages == 0 || processed < max_messages) &&
           xQueueReceive(queue, &message, 0) == pdTRUE) {
        adaptor->render(&message, adaptor->render_context);
        ++processed;
    }
    return processed;
}

esp_err_t dajilog_ui_adaptor_submit(const dajilog_ui_msg_t *command)
{
    return dajilog_ui_bridge_send_command(command);
}
