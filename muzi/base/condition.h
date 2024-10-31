#ifndef MUZI_BASE_CONDITION_H_
#define MUZI_BASE_CONDITION_H_

#include <pthread.h>

#include "macros.h"
#include "mutex.h"
#include "noncopyable.h"

namespace muzi
{
class Condition : noncopyable
{
public:
    Condition(MutexLock &lock) : lock_(lock)
    {
        ISZERO(pthread_cond_init(&cond_, NULL));
    }

    ~Condition()
    {
        ISZERO(pthread_cond_destroy(&cond_));
    }

    void Wait() const
    {
        MutexLock::HolderGuard hg(lock_);
        ISZERO(pthread_cond_wait(&cond_, lock_.GetPthreadMutex()));
    }

    void Notify() const
    {
        ISZERO(pthread_cond_signal(&cond_));
    }

    void NotifyAll() const
    {
        ISZERO(pthread_cond_broadcast(&cond_));
    }

    bool WaitForSeconds(double seconds) const;

private:
    MutexLock &lock_;
    mutable pthread_cond_t cond_;
};
}   // namespace muzi

#endif  // MUZI_BASE_CONDITION_H_
