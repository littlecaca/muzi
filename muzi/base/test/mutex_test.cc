#include <iostream>
#include <time.h>

#include <pthread.h>

#include "mutex.h"

long long cnt_unsafe = 1;
long long cnt = 1;

constexpr int iter_cnt = 500;
muzi::MutexLock mutex;
timespec ts;

void *add_unsafe(void *)
{
    for (int i = 0; i < iter_cnt; ++i)
    {
        ::nanosleep(&ts, NULL);
        cnt_unsafe += 1;
    }
    return NULL;
}

void *add(void *)
{
    for (int i = 0; i < iter_cnt; ++i)
    {
        ::nanosleep(&ts, NULL);
        muzi::MutexLockGuard lock_guard(mutex);
        cnt += 1;
    }
    return NULL;
}

void *substract_unsafe(void *)
{
    for (int i = 0; i < iter_cnt; ++i)
    {
        ::nanosleep(&ts, NULL);
        cnt_unsafe -= 1;
    }
    return NULL;
}

void *substract(void *)
{
    for (int i = 0; i < iter_cnt; ++i)
    {
        ::nanosleep(&ts, NULL);
        muzi::MutexLockGuard lock_guard(mutex);
        cnt -= 1;
    }
    return NULL;
}



int main(int argc, char const *argv[])
{
    std::cout << "Test starting ..." << std::endl;
    ts.tv_nsec = 1000 * 1000;
    ts.tv_sec = 0;

    pthread_t t1, t2, t3, t4;
    pthread_create(&t1, NULL, add_unsafe, NULL);
    pthread_create(&t2, NULL, add, NULL);
    pthread_create(&t3, NULL, substract_unsafe, NULL);
    pthread_create(&t4, NULL, substract, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    std::cout << "cnt_unsafe: " << cnt_unsafe << "\n";
    std::cout << "cnt: " << cnt << std::endl;
     
    return 0;
}
