
#ifndef IOBSERVER_H
#define IOBSERVER_H

#include <vector>
#include "SubjectType.h"

// 기본 Observer 인터페이스
class IObserver {
public:
    virtual ~IObserver() = default;
    // data: 전달되는 데이터, subjectId: SubjectType을 정수형으로 전달
    virtual void onNotify(int data, int subjectId) = 0;
};

// ISubscriber 인터페이스는 IObserver를 확장하여, Observer가 관심 있는 SubjectType 목록을 제공
class ISubscriber : public IObserver {
public:
    virtual std::vector<SubjectType> getSubscribedSubjectTypes() const = 0;
};

#endif  // IOBSERVER_H
