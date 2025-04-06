
#ifndef QUEUE_MEDIATOR_H
#define QUEUE_MEDIATOR_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <vector>
#include "ITask.h"

struct MediatorMessage {
    int msg;
    ITask* target;  // nullptr이면 모든 구독자에게 전달
    int param;      // 추가 데이터 (예: 센서 값 등)
};
class QueueMediator {
public:
    QueueMediator();
    ~QueueMediator();

    // 큐를 생성하고 dispatcher 태스크를 시작 (queueLength: 큐 항목 수)
    bool init(size_t queueLength);

    // 메시지를 큐에 발행(target이 nullptr이면 브로드캐스트)
    void publish(int msg, int param = 0, ITask* target = nullptr);

    // 구독자 등록 (메시지를 받고자 하는 Task)
    void subscribe(ITask* subscriber);

    // 구독자 해제
    void unsubscribe(ITask* subscriber);

private:
    // dispatcher 태스크 함수 (static 함수로 FreeRTOS 태스크 생성 시 사용)
    static void dispatcherTask(void* param);
    // 실제 메시지 분배 루프
    void dispatchLoop();

    QueueHandle_t messageQueue;
    std::vector<ITask*> subscribers;
};

#endif  // QUEUE_MEDIATOR_H
