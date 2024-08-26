#include <stdio.h>
#include <memory>

#include <unistd.h>

#include "current_thread.h"
#include "event_loop.h"
#include "thread.h"
#include "countdown_latch.h"

muzi::EventLoop *g_loop;

/// @brief Test calling Loop in other threads
/// @return This test is expected to abort

void ThreadFunc(muzi::CountdownLatch *latch)
{
    printf("ThreadFunc(): pid = %d, tid = %d\n",
        ::getpid(), muzi::current_thread::tid());
    ::fflush(stdout);
    auto loop = new muzi::EventLoop;
    g_loop = loop;
    latch->CountDown();
}

int main(int argc, char const *argv[])
{
    muzi::CountdownLatch latch(1);
    muzi::Thread t1(std::bind(ThreadFunc, &latch), "t1");
    t1.Start();
    latch.Wait();
    
    g_loop->Loop();

    delete g_loop;
    return 0;
}
