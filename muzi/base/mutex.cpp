#include "muzi/base/mutex.h"

using namespace muzi;

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

MutexLock::MutexAttr  MutexLock::mutex_attr_;

MutexLock::MutexLock()
{
    pthread_mutex_init(&mutex_, &mutex_attr_.mutex_attr);
}
