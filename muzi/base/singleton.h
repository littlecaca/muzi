/**
 * Singleton
 * [status] done
 */
#ifndef MUZI_BASE_SINGLETON_H_
#define MUZI_BASE_SINGLETON_H_

#include <stdlib.h> // atexit

#include <pthread.h>

#include "noncopyable.h"

namespace muzi {
template <typename T>
class Singleton : noncopyable
{
public:
    static T &getInstance()
    {
        pthread_once(&ponce_, init);
        return *pobj_;
    }

    Singleton() = delete;
    ~Singleton() = delete;

private:
    static void init()
    {
        atexit(&clear);
        pobj_ = new T();
    }
    
    static void clear()
    {
        delete pobj_;
    }

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
