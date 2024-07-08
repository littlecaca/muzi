#ifndef MUZI_BASE_MUTEX_H_
#define MUZI_BASE_MUTEX_H_

#include <cassert>

#include <pthread.h>

#include "muzi/base/noncopyable.h"
#include "muzi/debug.h"

// Indicate that "the dead lock will not be examined"
#define MUTEX_LOCK_ATTR PTHREAD_MUTEX_NORMAL


namespace muzi {

/**
 * MutexLock
 */
class MutexLock : muzi::noncopyable
{
public:
    MutexLock();
    ~MutexLock()
    {
        assert(holder_ == 0);
        pthread_mutex_destroy(&mutex_);
    }

    // bool isLockedByThisThread() { return holder_ == C}

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
    class MutexAttr;
    static MutexAttr mutex_attr_;
};

}   // namespace muzi

#endif // MUZI_BASE_MUTEX_H_
