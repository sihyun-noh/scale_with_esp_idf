
#pragma once
#include "ITask.h"
#include "IObserver.h"
#include "SubjectType.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char* TAG_LOGGING = "LoggingTask";

class LoggingTask : public ITask, public IObserver {
public:
    LoggingTask(const std::vector<SubjectType>& subs) : subscribedSubjects(subs), taskName(TAG_LOGGING) {}
    virtual ~LoggingTask() = default;

    void run() override {
        while (true) {
            ESP_LOGI(TAG_LOGGING, "LoggingTask is active...");
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }

    void onNotify(int data) override { ESP_LOGI(TAG_LOGGING, "Logging sensor data: %d", data); }

    void handleMessage(int msg) override { ESP_LOGI(TAG_LOGGING, "LoggingTask handleMessage: %d", msg); }

    TaskType getTaskType() const override { return TaskType::LOGGING; }
    const char* getName() const override { return name; }

private:
    std::vector<SubjectType> subscribedSubjects;
    const char* taskName;
};
