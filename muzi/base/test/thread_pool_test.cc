#include "thread_pool.h"

#include <stdio.h>

#include <sys/time.h>

#include "logger.h"

using namespace muzi;
using namespace std;


void ThreadFunc1()
{
    for (int i = 0; i < 1000; ++i)
    {
        if (i % 7 == 2)
        {
            ::usleep(1000 * 2); // 2ms
            printf("%d\n", i);
        }
    }
}

void ThreadFunc2()
{
    for (int i = 0; i < 100; ++i)
    {
        printf("get it!\n");
        ::usleep(1000 * 2);
    }
}

int main(int argc, char const *argv[])
{
    LOG_INFO << "Start thread_pool test";

    ThreadPool pool;
    pool.Start(2);
    pool.Run(&ThreadFunc1);
    pool.Run(&ThreadFunc2);

    ::usleep(1000 * 10);    // 10ms
    return 0;
}
