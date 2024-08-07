#include "condition.h"

#include <errno.h>
#include <time.h>

#include "timestamp.h"

namespace muzi
{
bool Condition::WaitForSeconds(double seconds)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += static_cast<int>(seconds);
    ts.tv_nsec += static_cast<int>((seconds - static_cast<int>(seconds)) 
        * Timestamp::kNanosecondsPerSecond);
    
    MutexLock::HolderGuard holder(lock_);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, lock_.GetPthreadMutex(), &ts);
}
}   // namespace muzi
