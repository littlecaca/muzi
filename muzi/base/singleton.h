#ifndef MUZI_BASE_SINGLETON_H_
#define MUZI_BASE_SINGLETON_H_

#include <pthread.h>

#include "noncopyable.h"

namespace muzi {
/**
 * Singleton
 * [status] done
 */
template <typename T>
class Singleton : noncopyable
{
public:
    static T &instance()
    {
        pthread_once(&ponce_, &init);
        return *pobj_;
    }

    static void init()
    {
        pobj_ = new T();
    }

    Singleton() = delete;
    ~Singleton() = delete;

private:
    static pthread_once_t ponce_;
    static T* pobj_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::pobj_ = NULL;

}   // namespace muzi

#endif  // MUZI_BASE_SINGLETON_H_