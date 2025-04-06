
#ifndef ISUBJECT_H
#define ISUBJECT_H

#include "IObserver.h"

class ISubject {
public:
    virtual ~ISubject() = default;
    virtual void attach(IObserver* observer) = 0;
    virtual void detach(IObserver* observer) = 0;
    virtual void notify(int data) = 0;
};

#endif  // ISUBJECT_H
