
#ifndef GLOBAL_OBSERVER_TASK_H
#define GLOBAL_OBSERVER_TASK_H

#include "ITask.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

static const char* TAG_GLOBAL_OBSERVER = "GlobalObserverTask";
class SubjectTask;  // 전방 선언

class GlobalObserverTask : public ITask {
public:
    GlobalObserverTask();
    virtual ~GlobalObserverTask() = default;
    virtual void run() override;
    virtual void handleMessage(int msg) override;

    TaskType getTaskType() const override { return TaskType::GLOBAL_OBSERVER; }
    const char* getName() const override { return taskName; }

private:
    const char* taskName;
};

#endif  // GLOBAL_OBSERVER_TASK_H
