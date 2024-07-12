/**
 * MutexLock
 * [status] done
 * Implementing the RAII(Resourse Acquisition Is Initialization) style mutex lock.
 */
#ifndef MUZI_BASE_MUTEX_H_
#define MUZI_BASE_MUTEX_H_

#include <cassert>

#include <pthread.h>

#include "current_thread.h"
#include "macros.h"
#include "noncopyable.h"

// Indicate that "the dead lock will not be examined"
#define MUTEX_LOCK_ATTR PTHREAD_MUTEX_NORMAL

namespace muzi {

class MutexLock : noncopyable
{
    friend class MutexLockGuard;
public:
    MutexLock();
    ~MutexLock()
    {
        assert(holder_ == 0);
        ISZERO(pthread_mutex_destroy(&mutex_));
    }

    bool IsLockedByThisThread() const { return holder_ == current_thread::tid(); }

    void AssertLocked() const
    {
        assert(IsLockedByThisThread());
    }

private:
    // RAII: lock() and unlock() should only be not called by user code
    void Lock()
    {
        ISZERO(pthread_mutex_lock(&mutex_));
        holder_ = current_thread::tid();
    }

    void Unlock()
    {
        holder_ = 0;
        ISZERO(pthread_mutex_unlock(&mutex_));
    }

    pthread_mutex_t *GetPthreadMutex()
    {
        return &mutex_;
    }

private:
    pthread_mutex_t mutex_;
    pid_t holder_;
    class MutexAttr;
    static MutexAttr mutex_attr_;
};

class MutexLockGuard : noncopyable
{
public:
    explicit MutexLockGuard(MutexLock &mutex) : mutex_lock_(mutex)
    {
        mutex_lock_.Lock();
    }

    ~MutexLockGuard()
    {
        mutex_lock_.Unlock();
    }

private:
    MutexLock &mutex_lock_;
};


// A inter class of MutexLock to save the static pthread_mutexattr_t
class MutexLock::MutexAttr
{
public:
    MutexAttr()
    {
        ISZERO(pthread_mutexattr_init(&mutex_attr));
        ISZERO(pthread_mutexattr_settype(&mutex_attr, MUTEX_LOCK_ATTR));
    }
    ~MutexAttr()
    {
        ISZERO(pthread_mutexattr_destroy(&mutex_attr));
    }

    pthread_mutexattr_t mutex_attr;
};

inline MutexLock::MutexLock() : holder_(0)
{
    ISZERO(pthread_mutex_init(&mutex_, &mutex_attr_.mutex_attr));
}

}   // namespace muzi

#endif // MUZI_BASE_MUTEX_H_
