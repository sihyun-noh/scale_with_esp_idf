
#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ITask.h"
#include "TaskFactory.h"
#include "SubjectTask.h"
#include "QueueMediator.h"
#include "SubjectType.h"

class TaskManager {
public:
    // Singleton 인스턴스 접근
    static TaskManager& getInstance();

    // Observer Task 생성 (SUBJECT가 아닌 경우): 구독 목록을 전달
    bool createAndStartTask(TaskType type, const char* taskName, uint16_t stackSize = 4096, UBaseType_t priority = 5,
                            const std::vector<SubjectType>& subs = {});

    // SUBJECT Task 생성: SubjectType을 추가로 전달
    bool createAndStartTask(TaskType type, SubjectType subjectType, const char* taskName, uint16_t stackSize = 4096,
                            UBaseType_t priority = 5);

    // 글로벌 메시지 발행: target이 nullptr이면 브로드캐스트, 아니면 특정 Task에만 전달
    void sendGlobalMessage(int msg, ITask* target = nullptr);

    // 여러 Observer를 선택해서 메시지 발행
    void sendGlobalMessageToTargets(int msg, const std::vector<ITask*>& targets);

    // 자원 해제
    void cleanup();

    // TaskType을 반환
    ITask* getTaskByType(TaskType type) const;

    // 복수의 TaskType을 반환
    std::vector<ITask*> getTasksByType(TaskType type) const;

    // Global Observer Task에서 SubjectTask 목록 접근용
    std::vector<SubjectTask*> getSubjectTasks() const { return subjectTaskPtrs; }

private:
    TaskManager();
    ~TaskManager();
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;

    struct TaskInfo {
        ITask* taskObj;
        TaskHandle_t taskHandle;
    };

    std::vector<TaskInfo> tasks;
    std::vector<SubjectTask*> subjectTaskPtrs;
    QueueMediator mediator;
};

#endif  // TASKMANAGER_H
