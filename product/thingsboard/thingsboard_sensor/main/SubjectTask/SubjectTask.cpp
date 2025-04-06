
#include "SubjectTask.h"
#include "IStrategy.h"
#include "gpio_api.h"
#include "config.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
extern int sensor_init(void);
extern int sensor_read(void);
#ifdef __cplusplus
}
#endif

static int sensor_power_on() {
    return gpio_write(SENSOR_POWER_CONTROL_PORT, 1);
}
static int sensor_power_off() {
    return gpio_write(SENSOR_POWER_CONTROL_PORT, 0);
}

SubjectTask::SubjectTask(SubjectType type, IStrategy* strat)
    : strategy(strat), subjectType(type), subjectQueue(nullptr), taskName(TAG_SUBJECT) {
    // 각 SubjectTask마다 길이 10의 큐 생성 (메시지 크기: int)
    subjectQueue = xQueueCreate(10, sizeof(int));
    if (subjectQueue == nullptr) {
        ESP_LOGE(TAG_SUBJECT, "Failed to create queue for subject");
    }
}

SubjectTask::~SubjectTask() {
    if (strategy) {
        delete strategy;
        strategy = nullptr;
    }
    if (subjectQueue) {
        vQueueDelete(subjectQueue);
        subjectQueue = nullptr;
    }
}

void SubjectTask::setStrategy(IStrategy* strat) {
    if (strategy) {
        delete strategy;
    }
    strategy = strat;
}

void SubjectTask::attach(IObserver* observer) {
    ISubscriber* sub = dynamic_cast<ISubscriber*>(observer);
    if (sub) {
        subscribers.push_back(sub);
    }
}

void SubjectTask::detach(IObserver* observer) {
    ISubscriber* sub = dynamic_cast<ISubscriber*>(observer);
    if (sub) {
        subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), sub), subscribers.end());
    }
}

void SubjectTask::notify(int data) {
    // Observer들에게 현재 SubjectType과 함께 데이터 전달
    for (auto* sub : subscribers) {
        std::vector<SubjectType> subList = sub->getSubscribedSubjectTypes();
        if (std::find(subList.begin(), subList.end(), subjectType) != subList.end()) {
            sub->onNotify(data, static_cast<int>(subjectType));
        }
    }
}

void SubjectTask::run() {
    int value = 0;
    while (true) {
        // 전략이 설정되어 있다면 전략의 execute() 호출,
        // 없으면 기본 센서 읽기 동작 수행
        if (strategy) {
            strategy->execute();
        } else {
            ESP_LOGI(TAG_SUBJECT, "SENSOR_READ_MODE");
            // sensor_power_on();
            // // sensor warming up time
            // vTaskDelay(3000 / portTICK_PERIOD_MS);
            // if (sensor_read() == 0) {
            //     //                EventManager::GetInstance().SetEventBits(EVENT_GROUPS_FOR_THINGSBOARD,
            //     //                EVENT_BIT_TASK_READY);
            // } else {
            //     ESP_LOGE(TAG_SUBJECT, "sensor read, error = [%d]", 3);
            // }
            //
            // sensor_power_off();
        }
        value++;
        ESP_LOGI(TAG_SUBJECT, "Subject[%d] value = %d", static_cast<int>(subjectType), value);
        if (subjectQueue) {
            xQueueSend(subjectQueue, &value, 0);
        }
        // 데이터 변경 시 Observer들에게 알림
        notify(value);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
