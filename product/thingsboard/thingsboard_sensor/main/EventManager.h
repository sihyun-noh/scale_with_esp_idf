#ifndef _EVENT_MANAGER_H_
#define _EVENT_MANAGER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <unordered_map>
#include <string>

#define EVENT_GROUPS_FOR_THINGSBOARD "thingsboard"
#define EVENT_BIT_TASK_READY (1 << 0)

class EventManager {
public:
    // 싱글턴 인스턴스 가져오기
    static EventManager& GetInstance();

    // 🎯 특정 Event Group 가져오기 (없으면 생성)
    EventGroupHandle_t GetEventGroup(const std::string& name);

    // 이벤트 비트 설정
    void SetEventBits(const std::string& name, EventBits_t bits);

    // 이벤트 비트 대기
    EventBits_t WaitForEventBits(const std::string& name, EventBits_t bits, TickType_t timeout = portMAX_DELAY);

    // 특정 Event Group의 이벤트 비트 초기화
    void ClearEventBits(const std::string& name, EventBits_t bits);

    // Event Group 삭제
    void RemoveEventGroup(const std::string& name);

private:
    EventManager();   // 생성자 (private → 싱글턴)
    ~EventManager();  // 소멸자

    std::unordered_map<std::string, EventGroupHandle_t> eventGroups;  // 🎯 여러 EventGroup 관리

    // 복사 & 할당 방지 (싱글턴 패턴)
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;
};

#endif
