#include <iostream>

#include <pthread.h>

#include "condition.h"
#include "mutex.h"

int sum = 100;
constexpr int kIterCnt = 500;
muzi::MutexLock lock;
muzi::Condition cond(lock);

void *sub(void *)
{
    muzi::MutexLockGuard mlg(lock);
    for (int i = 0; i  < kIterCnt; ++i)
    {
        while (sum & 1)
        {
            cond.Notify();
            cond.Wait();
        }
        ++sum;
    }
    cond.Notify();
    std::cout << "done sub sum: " << sum  << std::endl;
    return NULL;
}

void *add(void *)
{
    muzi::MutexLockGuard mlg(lock);
    for (int i = 0; i < kIterCnt; ++i)
    {
        while (!(sum & 1))
        {
            cond.Notify();
            cond.Wait();
        }
        --sum;
    }
    cond.Notify();
    std::cout << "done add sum " << sum << std::endl;

    return NULL;
}


int main(int argc, char const *argv[])
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, sub, NULL);
    pthread_create(&t2, NULL, add, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    assert(sum == 100);

    return 0;
}
