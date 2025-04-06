
#include "GlobalObserverTask.h"
#include "TaskManager.h"  // TaskManager에서 SubjectTask 목록을 얻기 위해 사용
#include "SubjectTask.h"  // 수정: SensorTask 대신 SubjectTask 사용
#include "freertos/queue.h"
#include "esp_log.h"
#include <vector>

GlobalObserverTask::GlobalObserverTask() : taskName(TAG_GLOBAL_OBSERVER) {
    ESP_LOGI(TAG_GLOBAL_OBSERVER, "GlobalObserverTask initialized");
}
void GlobalObserverTask::run() {
    while (true) {
        // TaskManager에서 등록된 모든 SubjectTask의 큐를 폴링
        std::vector<SubjectTask*> subjects = TaskManager::getInstance().getSubjectTasks();
        for (auto subject : subjects) {
            if (subject) {
                QueueHandle_t q = subject->getQueue();
                int subjectData = 0;
                // 각 Subject의 큐에서 non-blocking 방식으로 메시지를 읽음
                if (xQueueReceive(q, &subjectData, 0) == pdTRUE) {
                    ESP_LOGI(TAG_GLOBAL_OBSERVER, "Received data %d from Subject [%d]", subjectData,
                             static_cast<int>(subject->getSubjectType()));
                    // 필요한 추가 처리 수행 가능
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));  // 짧은 지연으로 바쁜 루프 방지
    }
}

void GlobalObserverTask::handleMessage(int msg) {
    ESP_LOGI(TAG_GLOBAL_OBSERVER, "GlobalObserverTask handleMessage: %d", msg);
}
