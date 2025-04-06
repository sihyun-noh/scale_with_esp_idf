
#ifndef SUBJECT_TASK_H
#define SUBJECT_TASK_H

#include "ITask.h"
#include "ISubject.h"
#include "IObserver.h"
#include "SubjectType.h"
#include "IStrategy.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <vector>
#include <algorithm>

static const char* TAG_SUBJECT = "SubjectTask";

class SubjectTask : public ITask, public ISubject {
public:
    SubjectTask(SubjectType type, IStrategy* strat = nullptr);
    virtual ~SubjectTask();

    virtual void run() override;
    virtual void setStrategy(IStrategy* strat);  // 런타임 전략 변경
    virtual void handleMessage(int msg) override {}

    // ISubject 인터페이스
    virtual void attach(IObserver* observer) override;
    virtual void detach(IObserver* observer) override;
    virtual void notify(int data) override;

    TaskType getTaskType() const override { return TaskType::SUBJECT; }
    const char* getName() const override { return taskName; }

    QueueHandle_t getQueue() const { return subjectQueue; }
    SubjectType getSubjectType() const { return subjectType; }

private:
    IStrategy* strategy;  // 실행할 strategy
    SubjectType subjectType;
    QueueHandle_t subjectQueue;
    std::vector<ISubscriber*> subscribers;
    const char* taskName;
};

#endif  // SUBJECT_TASK_H
