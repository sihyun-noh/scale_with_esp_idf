#ifndef ISTRATEGY_H
#define ISTRATEGY_H

class IStrategy {
public:
    virtual void execute() = 0;  // 순수 가상 함수: 각 전략별 실행 로직
    virtual ~IStrategy() {}
};

#endif  // ISTRATEGY_H
