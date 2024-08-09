#include "timer_queue.h"

#include <iostream>
#include <string>

#include "countdown_latch.h"
#include "event_loop.h"
#include "thread.h"
#include "logger.h"

muzi::EventLoop *g_loop;

void ThreadFunc(muzi::CountdownLatch *latch)
{
    muzi::EventLoop loop;
    g_loop = &loop;
    latch->CountDown();

    g_loop->Loop();
}

void print(const std::string &msg)
{
    LOG_INFO << msg;
}

int main(int argc, char const *argv[])
{
    muzi::gDefaultLogger.SetLogLevel(muzi::kInfo);
    muzi::CountdownLatch latch(1);
    
    muzi::Thread t(std::bind(ThreadFunc, &latch), "");
    t.Start();
    latch.Wait();

    LOG_INFO << "Now is " << muzi::Timestamp().ToFormatString();
    g_loop->RunAfter(1, std::bind(print, "I run after 1 second"));
    g_loop->RunAfter(1.2, std::bind(print, "I run after 1.2 second"));
    g_loop->RunAfter(1.44, std::bind(print, "I run after 1.44 second"));

    g_loop->RunEvery(muzi::Timestamp(), std::bind(print, "Repeated Run"), 0.5);

    g_loop->RunAt(muzi::Timestamp().AddTime(0.6), std::bind(print, "RunAt"));

    sleep(2);
    g_loop->Quit();
    t.Join();
    return 0;
}
