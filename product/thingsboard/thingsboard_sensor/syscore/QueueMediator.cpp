
#include "QueueMediator.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <algorithm>

static const char* TAG_MEDIATOR = "QueueMediator";

QueueMediator::QueueMediator() : messageQueue(nullptr) {}

QueueMediator::~QueueMediator() {
    if (messageQueue) {
        vQueueDelete(messageQueue);
    }
}

bool QueueMediator::init(size_t queueLength) {
    messageQueue = xQueueCreate(queueLength, sizeof(MediatorMessage));
    if (messageQueue == nullptr) {
        ESP_LOGE(TAG_MEDIATOR, "Failed to create message queue");
        return false;
    }
    BaseType_t result = xTaskCreate(dispatcherTask, "MediatorDispatcher", 4096, this, 5, NULL);
    if (result != pdPASS) {
        ESP_LOGE(TAG_MEDIATOR, "Failed to create dispatcher task");
        return false;
    }
    return true;
}

void QueueMediator::publish(int msg, int param, ITask* target) {
    if (messageQueue) {
        MediatorMessage mMsg = { msg, target, param };
        xQueueSend(messageQueue, &mMsg, 0);
    }
}

void QueueMediator::subscribe(ITask* subscriber) {
    subscribers.push_back(subscriber);
}

void QueueMediator::unsubscribe(ITask* subscriber) {
    subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), subscriber), subscribers.end());
}

void QueueMediator::dispatcherTask(void* param) {
    QueueMediator* mediator = static_cast<QueueMediator*>(param);
    mediator->dispatchLoop();
}

void QueueMediator::dispatchLoop() {
    MediatorMessage mMsg;
    while (true) {
        if (xQueueReceive(messageQueue, &mMsg, portMAX_DELAY) == pdTRUE) {
            if (mMsg.target == nullptr) {
                for (auto* sub : subscribers) {
                    sub->handleMessage(mMsg.msg);
                }
            } else {
                for (auto* sub : subscribers) {
                    if (sub == mMsg.target) {
                        sub->handleMessage(mMsg.msg);
                        break;
                    }
                }
            }
        }
    }
}
