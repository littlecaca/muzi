/**
 * MutexLock
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
    friend class Condition;
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
    // RAII: lock() and unlock() should only be called by MutexLockGuard
    void Lock()
    {
        ISZERO(pthread_mutex_lock(&mutex_));
        AssignHolder();
    }

    void Unlock()
    {
        UnassignHolder();
        ISZERO(pthread_mutex_unlock(&mutex_));
    }

    // Should only be called by Condition
    pthread_mutex_t *GetPthreadMutex()
    {
        return &mutex_;
    }

    // RAII: assign and unassign holder_
    // Should only be called by Condition
    class HolderGuard : noncopyable
    {
    public:
        explicit HolderGuard(MutexLock &owner) : owner_(owner)
        {
            owner_.UnassignHolder();
        }

        ~HolderGuard()
        {
            owner_.AssignHolder();
        }

    private:
        MutexLock &owner_;
    };

    void AssignHolder()
    {
        holder_ = current_thread::tid();
    }

    void UnassignHolder()
    {
        holder_ = 0;
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

#define MutexLockGuard(x) [Wrong Usage of MutexLockGuard)

// A inter class of MutexLock to save the static pthread_mutexattr_t
class MutexLock::MutexAttr
{
public:
    MutexAttr();
    ~MutexAttr();

    pthread_mutexattr_t mutex_attr;
};

inline MutexLock::MutexLock() : holder_(0)
{
    ISZERO(pthread_mutex_init(&mutex_, &mutex_attr_.mutex_attr));
}

}   // namespace muzi
// Macro annotations which just for clang and tipping programer
#if defined(__clang__) && !defined(SWIG)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)
#endif

#define CAPABILITY(x)   \
    THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define GUARDED_BY(x)   \
    THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#endif // MUZI_BASE_MUTEX_H_
