#ifndef STRATEGY1_H
#define STRATEGY1_H

#include "IStrategy.h"

class Strategy1 : public IStrategy {
public:
    void execute() override;
private:
    static bool isInitialized;  // 초기화 상태를 추적하는 정적 변수
};

#endif  // STRATEGY1_H
