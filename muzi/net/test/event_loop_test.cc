#include <stdio.h>

#include <unistd.h>

#include "current_thread.h"
#include "event_loop.h"
#include "thread.h"

void ThreadFunc()
{
    printf("ThreadFunc(): pid = %d, tid = %d\n",
        ::getpid(), muzi::current_thread::tid());
    muzi::EventLoop loop;
    loop.loop();
}

int main(int argc, char const *argv[])
{
    muzi::Thread t1(ThreadFunc, "t1"), t2(ThreadFunc, "t2");
    t1.Start();
    t2.Start();
    t1.Join();
    t2.Join();

    return 0;
}
