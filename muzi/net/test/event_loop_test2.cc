#include <stdio.h>
#include <memory>

#include <unistd.h>

#include "current_thread.h"
#include "event_loop.h"
#include "thread.h"

muzi::EventLoop *g_loop;

// This is a negative test

void ThreadFunc()
{
    printf("ThreadFunc(): pid = %d, tid = %d\n",
        ::getpid(), muzi::current_thread::tid());
    auto loop = new muzi::EventLoop;
    g_loop = loop;
    loop->Loop();
}

int main(int argc, char const *argv[])
{
    muzi::Thread t1(ThreadFunc, "t1");
    t1.Start();
    t1.Join();
    g_loop->Loop();

    delete g_loop;

    return 0;
}