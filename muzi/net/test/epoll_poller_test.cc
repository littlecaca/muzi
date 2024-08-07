#include <sys/timerfd.h>
#include <unistd.h>

#include "channel.h"
#include "event_loop.h"
#include "logger.h"
#include "poller.h"


muzi::EventLoop *g_loop;

void timeout()
{
    LOG_TRACE << "timeout";
    g_loop->Quit();
}

int main(int argc, char const *argv[])
{
    muzi::EventLoop loop;
    g_loop = &loop;

    int timer_fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    muzi::Channel channel(&loop, timer_fd);

    channel.SetReadCallback(timeout);
    channel.EnableReading();

    struct itimerspec howlong;
    memset(&howlong, 0, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timer_fd, 0, &howlong, NULL);

    loop.Loop();

    ::close(timer_fd);

    return 0;
}
