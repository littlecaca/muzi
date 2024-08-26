#include "event_loop_thread.h"

#include <iostream>

#include "logger.h"


void print(const std::string &msg)
{
    LOG_INFO << msg;
}

int main(int argc, char const *argv[])
{
    muzi::gDefaultLogger.SetLogLevel(muzi::kInfo);

    muzi::EventLoopThread event_thread;
    
    muzi::EventLoop *loop = event_thread.StartLoop();

    LOG_INFO << "Now is " << muzi::Timestamp().ToFormatString();

    loop->RunAfter(1, std::bind(print, "I run after 1 second"));
    loop->RunAfter(1.2, std::bind(print, "I run after 1.2 second"));
    loop->RunAfter(1.44, std::bind(print, "I run after 1.44 second"));
    loop->RunEvery(muzi::Timestamp(), std::bind(print, "Repeated Run"), 0.5);
    loop->RunAt(muzi::Timestamp().AddTime(0.6), std::bind(print, "RunAt"));

    sleep(2);
    
    return 0;
}
