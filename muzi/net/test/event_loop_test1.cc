#include <stdio.h>

#include <unistd.h>

#include "current_thread.h"
#include "event_loop.h"
#include "logger.h"
#include "thread.h"
#include "countdown_latch.h"

/// @brief Test Starting and closing EventLoop

muzi::EventLoop *g_loop;

void ThreadFunc(muzi::CountdownLatch *latch)
{
    LOG_TRACE << "ThreadFunc(): pid = " << ::getpid()
              <<  " thread_id = " << muzi::current_thread::tid() << "\n";
    muzi::EventLoop loop;
    g_loop = &loop;

    latch->CountDown();
    loop.Loop();
}

int main(int argc, char const *argv[])
{
    muzi::CountdownLatch latch(1);
    muzi::Thread t1(std::bind(ThreadFunc, &latch), "t1");
    t1.Start();
    latch.Wait();
    g_loop->Quit();
    t1.Join();

    return 0;
}
