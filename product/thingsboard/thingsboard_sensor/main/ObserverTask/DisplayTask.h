
// #pragma once
//
// #include <stdio.h>
// #include "ITask.h"
// #include "IObserver.h"
// #include "esp_log.h"
//
// static const char* TAG_DISPLAY = "DisplayTask";
//
// class DisplayTask : public ITask, public IObserver {
// public:
//     void run() override {
//         while (true) {
//             // 디스플레이에서 할 일
//             ESP_LOGI(TAG_DISPLAY, "DisplayTask updating screen...");
//
//             vTaskDelay(pdMS_TO_TICKS(1000));
//         }
//     }
//
//     void onNotify(int data) override {
//         // Sensor값이 갱신되면 디스플레이에 표시
//         ESP_LOGI(TAG_DISPLAY, "Display shows sensor data: %d", data);
//     }
// };
//

#ifndef DISPLAY_TASK_H
#define DISPLAY_TASK_H

#include "ITask.h"
#include "IObserver.h"
#include "SubjectType.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

static const char* TAG_DISPLAY = "DisplayTask";

class DisplayTask : public ITask, public ISubscriber {
public:
    DisplayTask(const std::vector<SubjectType>& subs);
    virtual ~DisplayTask() = default;

    virtual void run() override;
    virtual void handleMessage(int msg) override;

    virtual std::vector<SubjectType> getSubscribedSubjectTypes() const override;
    virtual void onNotify(int data, int subjectId) override;

    TaskType getTaskType() const override { return TaskType::DISPLAY; }
    const char* getName() const override { return taskName; }

private:
    std::vector<SubjectType> subscribedSubjects;
    const char* taskName;
};

#endif  // DISPLAY_TASK_H
