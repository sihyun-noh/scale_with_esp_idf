
#ifndef ITASK_H
#define ITASK_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

enum class TaskType { SUBJECT, CONTROLLER, DISPLAY, LOGGING, NETWORK, GLOBAL_OBSERVER };

class ITask {
public:
    virtual ~ITask() = default;
    virtual void run() = 0;
    virtual void handleMessage(int msg) = 0;

    // TaskType 식별용
    virtual TaskType getTaskType() const = 0;
    // TaskName 식별용
    virtual const char* getName() const = 0;
};
#endif  // ITASK_H
