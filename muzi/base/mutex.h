#ifndef MUZI_BASE_MUTEX_H_
#define MUZI_BASE_MUTEX_H_

#include <cassert>

#include <pthread.h>

#include "noncopyable.h"
#include "debug.h"

// Indicate that "the dead lock will not be examined"
#define MUTEX_LOCK_ATTR PTHREAD_MUTEX_NORMAL

namespace muzi {

/**
 * MutexLock
 * [status] working
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

    // bool isLockedByThisThread() { return holder_ == }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
    class MutexAttr;
    static MutexAttr mutex_attr_;
};

class MutexLock::MutexAttr
{
public:
    MutexAttr()
    {
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_settype(&mutex_attr, MUTEX_LOCK_ATTR);
    }
    ~MutexAttr()
    {
        pthread_mutexattr_destroy(&mutex_attr);
    }

    pthread_mutexattr_t mutex_attr;
};

inline MutexLock::MutexLock() : holder_(0)
{
    pthread_mutex_init(&mutex_, &mutex_attr_.mutex_attr);
}

}   // namespace muzi

#endif // MUZI_BASE_MUTEX_H_
