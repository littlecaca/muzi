#include "countdown_latch.h"

namespace muzi
{
void CountdownLatch::CountDown()
{
    MutexLockGuard guard(mutex_);
    --count_;
    if (count_ == 0)
        cond_.NotifyAll();
}

void CountdownLatch::Wait()
{
    MutexLockGuard guard(mutex_);
    while (count_ > 0)
        cond_.Wait();
}

int CountdownLatch::GetCount()
{
    MutexLockGuard guard(mutex_);
    return count_;
}

}   // namespace muzi
