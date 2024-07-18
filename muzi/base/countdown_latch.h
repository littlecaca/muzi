#ifndef MUZI_BASE_COUNTDOWN_LATCH_H_
#define MUZI_BASE_COUNTDOWN_LATCH_H_

#include "condition.h"
#include "mutex.h"

namespace muzi
{
class CountdownLatch
{
public:
    explicit CountdownLatch(int count) : count_(count), cond_(mutex_) {}

    void CountDown();

    void Wait();

    int GetCount();

private:
    MutexLock mutex_;
    int count_ GUARDED_BY(mutex_);
    Condition cond_ GUARDED_BY(mutex_);
};

}   // namespace muzi

#endif  // MUZI_BASE_COUNTDOWN_LATCH_H_
