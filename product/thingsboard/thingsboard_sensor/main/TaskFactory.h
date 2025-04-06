
#ifndef TASKFACTORY_H
#define TASKFACTORY_H

#include "ITask.h"
#include "Strategy1.h"
#include "Strategy2.h"
#include "SubjectTask.h"
#include "ControllerTask.h"
#include "DisplayTask.h"
// #include "LoggingTask.h"  // LoggingTask 유사하게 구현 가능
#include "NetworkTask.h"
#include "GlobalObserverTask.h"
#include "SubjectType.h"
#include <vector>

// enum class TaskType { SUBJECT, CONTROLLER, DISPLAY, LOGGING, NETWORK, GLOBAL_OBSERVER };

class TaskFactory {
public:
    // SUBJECT 타입: 추가 파라미터 SubjectType 사용
    static ITask* createTask(TaskType type, SubjectType subjectType) {
        switch (subjectType) {
            case SubjectType::SENSOR1: return new SubjectTask(SubjectType::SENSOR1, new Strategy1());
            case SubjectType::SENSOR2:
                return new SubjectTask(SubjectType::SENSOR2, new Strategy2());
                //            case TaskType::SUBJECT: return new SubjectTask(subjectType);
            default: return nullptr;
        }
    }

    // Observer 타입: 생성 시 구독 목록(vector<SubjectType>) 전달
    static ITask* createTask(TaskType type, const std::vector<SubjectType>& subs) {
        switch (type) {
            case TaskType::CONTROLLER: return new ControllerTask(subs);
            case TaskType::DISPLAY: return new DisplayTask(subs);
            // case TaskType::LOGGING:
            //     return new LoggingTask(subs);
            case TaskType::NETWORK: return new NetworkTask(subs);
            default: return nullptr;
        }
    }

    // GLOBAL_OBSERVER: 별도 파라미터 없이 생성
    static ITask* createTask(TaskType type) {
        switch (type) {
            case TaskType::GLOBAL_OBSERVER: return new GlobalObserverTask();
            default: return nullptr;
        }
    }
};

#endif  // TASKFACTORY_H
