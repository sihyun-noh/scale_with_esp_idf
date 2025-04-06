
// #include "TaskManager.h"
// #include <algorithm>
// #include "esp_log.h"
//
// static const char* TAG_MANAGER = "TaskManager";
//
// TaskManager& TaskManager::getInstance() {
//     static TaskManager instance;
//     return instance;
// }
//
// TaskManager::TaskManager() {
//     mediator.init(10);
// }
//
// TaskManager::~TaskManager() {
//     cleanup();
// }
//
// bool TaskManager::createAndStartTask(TaskType type, const char* taskName, uint16_t stackSize, UBaseType_t priority,
//                                      const std::vector<SubjectType>& subs) {
//     ITask* taskObj = TaskFactory::createTask(type, subs);
//     if (!taskObj) {
//         ESP_LOGE(TAG_MANAGER, "Failed to create task object for type %d", static_cast<int>(type));
//         return false;
//     }
//     // Observer Task는 글로벌 메시지를 받기 위해 mediator에 구독 등록
//     if (type != TaskType::SUBJECT) {
//         mediator.subscribe(taskObj);
//     }
//     TaskHandle_t handle;
//     BaseType_t result = xTaskCreate(
//         [](void* param) {
//             ITask* t = static_cast<ITask*>(param);
//             t->run();
//             vTaskDelete(NULL);
//         },
//         taskName, stackSize, taskObj, priority, &handle);
//     if (result == pdPASS) {
//         tasks.push_back({ taskObj, handle });
//         ESP_LOGI(TAG_MANAGER, "Task [%s] created.", taskName);
//         return true;
//     } else {
//         ESP_LOGE(TAG_MANAGER, "Failed to create task [%s].", taskName);
//         delete taskObj;
//         return false;
//     }
// }
//
// bool TaskManager::createAndStartTask(TaskType type, SubjectType subjectType, const char* taskName, uint16_t
// stackSize,
//                                      UBaseType_t priority) {
//     if (type != TaskType::SUBJECT) {
//         return createAndStartTask(type, taskName, stackSize, priority);
//     }
//     ITask* taskObj = TaskFactory::createTask(type, subjectType);
//     if (!taskObj) {
//         ESP_LOGE(TAG_MANAGER, "Failed to create SUBJECT task for subject type %d", static_cast<int>(subjectType));
//         return false;
//     }
//     SubjectTask* subjTask = dynamic_cast<SubjectTask*>(taskObj);
//     if (subjTask) {
//         subjectTaskPtrs.push_back(subjTask);
//     }
//     TaskHandle_t handle;
//     BaseType_t result = xTaskCreate(
//         [](void* param) {
//             ITask* t = static_cast<ITask*>(param);
//             t->run();
//             vTaskDelete(NULL);
//         },
//         taskName, stackSize, taskObj, priority, &handle);
//     if (result == pdPASS) {
//         tasks.push_back({ taskObj, handle });
//         ESP_LOGI(TAG_MANAGER, "Subject Task [%s] created.", taskName);
//         return true;
//     } else {
//         ESP_LOGE(TAG_MANAGER, "Failed to create Subject Task [%s].", taskName);
//         delete taskObj;
//         return false;
//     }
// }
//
// void TaskManager::sendGlobalMessage(int msg, ITask* target) {
//     mediator.publish(msg, 0, target);
// }
//
// void TaskManager::cleanup() {
//     for (auto& t : tasks) {
//         vTaskDelete(t.taskHandle);
//         delete t.taskObj;
//     }
//     tasks.clear();
//     subjectTaskPtrs.clear();
// }
//

#include "TaskManager.h"
#include <algorithm>
#include "esp_log.h"

static const char* TAG_MANAGER = "TaskManager";

TaskManager& TaskManager::getInstance() {
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager() {
    mediator.init(10);
}

TaskManager::~TaskManager() {
    cleanup();
}

// Observer Task 생성: SUBJECT가 아닌 경우
bool TaskManager::createAndStartTask(TaskType type, const char* taskName, uint16_t stackSize, UBaseType_t priority,
                                     const std::vector<SubjectType>& subs) {
    ITask* taskObj = TaskFactory::createTask(type, subs);
    if (!taskObj) {
        ESP_LOGE(TAG_MANAGER, "Failed to create task object for type %d", static_cast<int>(type));
        return false;
    }
    // // Observer Task는 글로벌 메시지를 받기 위해 mediator에 구독 등록
    // if (type != TaskType::SUBJECT) {
    //     mediator.subscribe(taskObj);
    //     // 자동으로 기존 SubjectTask에 attach() 시켜서 notify를 받도록 등록
    //     ISubscriber* subscriber = dynamic_cast<ISubscriber*>(taskObj);
    //     if (subscriber) {
    //         // 현재 생성된 모든 SubjectTask에 대해
    //         for (auto subject : subjectTaskPtrs) {
    //             // Observer가 구독하는 SubjectType 목록에 현재 Subject의 타입이 포함되어 있으면 attach
    //             std::vector<SubjectType> subList = subscriber->getSubscribedSubjectTypes();
    //             if (std::find(subList.begin(), subList.end(), subject->getSubjectType()) != subList.end()) {
    //                 subject->attach(taskObj);
    //             }
    //         }
    //     }
    // }

    // Observer Task는 글로벌 메시지를 받기 위해 mediator에 구독 등록
    if (type != TaskType::SUBJECT) {
        mediator.subscribe(taskObj);
        // Observer Task가 IObserver (또는 ISubscriber)를 구현했는지 확인 후,
        // 이미 생성된 모든 SubjectTask에 attach() 호출
        IObserver* observer = dynamic_cast<IObserver*>(taskObj);
        if (observer != nullptr) {
            for (auto subject : subjectTaskPtrs) {
                // Observer의 구독 목록에 현재 Subject의 타입이 포함되어 있으면 등록
                ISubscriber* subscriber = dynamic_cast<ISubscriber*>(observer);
                if (subscriber) {
                    std::vector<SubjectType> subList = subscriber->getSubscribedSubjectTypes();
                    if (std::find(subList.begin(), subList.end(), subject->getSubjectType()) != subList.end()) {
                        subject->attach(observer);
                    }
                }
            }
        }
    }

    TaskHandle_t handle;
    BaseType_t result = xTaskCreate(
        [](void* param) {
            ITask* t = static_cast<ITask*>(param);
            t->run();
            vTaskDelete(NULL);
        },
        taskName, stackSize, taskObj, priority, &handle);

    if (result == pdPASS) {
        tasks.push_back({ taskObj, handle });
        ESP_LOGI(TAG_MANAGER, "Task [%s] created.", taskName);
        return true;
    } else {
        ESP_LOGE(TAG_MANAGER, "Failed to create task [%s].", taskName);
        delete taskObj;
        return false;
    }
}

// SUBJECT Task 생성: SubjectType 추가 파라미터 사용
bool TaskManager::createAndStartTask(TaskType type, SubjectType subjectType, const char* taskName, uint16_t stackSize,
                                     UBaseType_t priority) {
    if (type != TaskType::SUBJECT) {
        // SUBJECT 타입이 아닌 경우는 위의 오버로드 함수로 호출
        return createAndStartTask(type, taskName, stackSize, priority);
    }
    ITask* taskObj = TaskFactory::createTask(type, subjectType);
    if (!taskObj) {
        ESP_LOGE(TAG_MANAGER, "Failed to create SUBJECT task for subject type %d", static_cast<int>(subjectType));
        return false;
    }
    // // SUBJECT Task는 별도 벡터에 저장(글로벌 Observer Task가 접근하기 위함)
    // SubjectTask* subjTask = dynamic_cast<SubjectTask*>(taskObj);
    // if (subjTask) {
    //     subjectTaskPtrs.push_back(subjTask);
    //     // 이미 생성된 Observer Task에 대해, 구독 목록에 해당 SubjectType이 포함되면 attach() 호출
    //     for (auto& ti : tasks) {
    //         ISubscriber* sub = dynamic_cast<ISubscriber*>(ti.taskObj);
    //         if (sub) {
    //             std::vector<SubjectType> list = sub->getSubscribedSubjectTypes();
    //             if (std::find(list.begin(), list.end(), subjTask->getSubjectType()) != list.end()) {
    //                 subjTask->attach(ti.taskObj);
    //             }
    //         }
    //     }
    // }
    //

    SubjectTask* subjTask = dynamic_cast<SubjectTask*>(taskObj);
    if (subjTask != nullptr) {
        subjectTaskPtrs.push_back(subjTask);
        // 이미 생성된 Observer Task들에 대해, 구독 목록에 해당 SubjectType이 포함되면 attach() 호출
        for (auto& ti : tasks) {
            IObserver* observer = dynamic_cast<IObserver*>(ti.taskObj);
            if (observer != nullptr) {
                ISubscriber* subscriber = dynamic_cast<ISubscriber*>(observer);
                if (subscriber) {
                    std::vector<SubjectType> list = subscriber->getSubscribedSubjectTypes();
                    if (std::find(list.begin(), list.end(), subjTask->getSubjectType()) != list.end()) {
                        subjTask->attach(observer);
                    }
                }
            }
        }
    }

    TaskHandle_t handle;
    BaseType_t result = xTaskCreate(
        [](void* param) {
            ITask* t = static_cast<ITask*>(param);
            t->run();
            vTaskDelete(NULL);
        },
        taskName, stackSize, taskObj, priority, &handle);

    if (result == pdPASS) {
        tasks.push_back({ taskObj, handle });
        ESP_LOGI(TAG_MANAGER, "Subject Task [%s] created.", taskName);
        return true;
    } else {
        ESP_LOGE(TAG_MANAGER, "Failed to create Subject Task [%s].", taskName);
        delete taskObj;
        return false;
    }
}

ITask* TaskManager::getTaskByType(TaskType type) const {
    for (const auto& t : tasks) {
        if (t.taskObj->getTaskType() == type) {
            return t.taskObj;
        }
    }
    return nullptr;
}

std::vector<ITask*> TaskManager::getTasksByType(TaskType type) const {
    std::vector<ITask*> result;
    for (const auto& t : tasks) {
        if (t.taskObj && t.taskObj->getTaskType() == type) {
            result.push_back(t.taskObj);
        }
    }
    return result;
}

void TaskManager::sendGlobalMessage(int msg, ITask* target) {
    mediator.publish(msg, 0, target);
}

void TaskManager::sendGlobalMessageToTargets(int msg, const std::vector<ITask*>& targets) {
    if (targets.empty()) {
        ESP_LOGW(TAG_MANAGER, "No targets provided for message %d", msg);
        return;
    }
    ESP_LOGI(TAG_MANAGER, "Sending message %d to %zu targets", msg, targets.size());

    for (size_t i = 0; i < targets.size(); ++i) {
        auto* task = targets[i];
        if (task) {
            ESP_LOGI("TaskManager", "→ Target[%zu]: %s (TaskType = %d)", i, task->getName(),
                     static_cast<int>(task->getTaskType()));
            // ESP_LOGI(TAG_MANAGER, "→ Target[%zu]: TaskType = %d", i, static_cast<int>(task->getTaskType()));
            mediator.publish(msg, 0, task);
        } else {
            ESP_LOGW(TAG_MANAGER, "Target[%zu] is nullptr", i);
        }
    }
}

void TaskManager::cleanup() {
    for (auto& t : tasks) {
        vTaskDelete(t.taskHandle);
        delete t.taskObj;
    }
    tasks.clear();
    subjectTaskPtrs.clear();
}
