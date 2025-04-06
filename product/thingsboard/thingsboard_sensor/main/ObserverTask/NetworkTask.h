

#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H
#include "ITask.h"
#include "IObserver.h"
#include "SubjectType.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <vector>

static const char* TAG_NETWORK = "NetworkTask";

class NetworkTask : public ITask, public ISubscriber {
public:
    NetworkTask(const std::vector<SubjectType>& subs);
    virtual ~NetworkTask() = default;

    virtual void run() override;
    virtual void handleMessage(int msg) override;

    virtual std::vector<SubjectType> getSubscribedSubjectTypes() const override;
    virtual void onNotify(int data, int subjectId) override;

    TaskType getTaskType() const override { return TaskType::NETWORK; }
    const char* getName() const override { return taskName; }

private:
    std::vector<SubjectType> subscribedSubjects;
    const char* taskName;
};

#endif
