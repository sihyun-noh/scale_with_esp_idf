//
// #pragma once
//
// #include <stdio.h>
// #include "ITask.h"
// #include "IObserver.h"
// #include "esp_log.h"
//
// static const char* TAG_CONTROLLER = "ControllerTask";
//
// class ControllerTask : public ITask, public IObserver {
// public:
//     // ITask 구현
//     void run() override {
//         while (true) {
//             // Controller는 내부적으로 뭔가 제어 로직을 수행
//             // ex) 명령 수신, 상태 확인 등
//             ESP_LOGI(TAG_CONTROLLER, "Controller running...");
//
//             // 500ms 대기
//             vTaskDelay(pdMS_TO_TICKS(500));
//         }
//     }
//
//     // Observer 구현
//     void onNotify(int data) override {
//         // SensorTask에서 온 데이터를 보고 제어 로직 수행
//         ESP_LOGI(TAG_CONTROLLER, "Controller notified of sensor data: %d", data);
//
//         // 예시) 특정 조건 시 TaskManager(혹은 다른 Task)로 메시지 보냄
//         // 실제 구현: handleMessage() 또는 Global Queue 사용 등
//     }
//
//     // 메시지 처리 (필요하다면)
//     void handleMessage(int msg) override { ESP_LOGI(TAG_CONTROLLER, "ControllerTask handleMessage: %d", msg); }
// };

#ifndef CONTROLLER_TASK_H
#define CONTROLLER_TASK_H

#include "ITask.h"
#include "IObserver.h"
#include "SubjectType.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

static const char* TAG_CONTROLLER = "ControllerTask";

class ControllerTask : public ITask, public ISubscriber {
public:
    ControllerTask(const std::vector<SubjectType>& subs);
    virtual ~ControllerTask() = default;

    virtual void run() override;
    virtual void handleMessage(int msg) override;

    virtual std::vector<SubjectType> getSubscribedSubjectTypes() const override;
    virtual void onNotify(int data, int subjectId) override;

    TaskType getTaskType() const override { return TaskType::CONTROLLER; }
    const char* getName() const override { return taskName; }

private:
    std::vector<SubjectType> subscribedSubjects;
    const char* taskName;
};

#endif  // CONTROLLER_TASK_H
