#include "macros.h"
#include "mutex.h"

namespace muzi {

MutexLock::MutexAttr::MutexAttr()
{
    ISZERO(pthread_mutexattr_init(&mutex_attr));
    ISZERO(pthread_mutexattr_settype(&mutex_attr, MUTEX_LOCK_ATTR));
}

MutexLock::MutexAttr::~MutexAttr()
{
    ISZERO(pthread_mutexattr_destroy(&mutex_attr));
}

MutexLock::MutexAttr MutexLock::mutex_attr_;

}   // namespace muzi
