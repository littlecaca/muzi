#ifndef MUZI_NET_EVENTLOOP_H_
#define MUZI_NET_EVENTLOOP_H_

#include <atomic>
#include <pthread.h>

#include "current_thread.h"
#include "noncopyable.h"

namespace muzi
{
class Poller;

// Implementing "One loop per thread"
class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void AssertInLoopThread()
    {
        AbortNotInLoopThread();
    }

    bool IsInLoopThread() const
    {
        return tid_ == current_thread::tid();
    }

    static EventLoop *GetLoopOfCurrentThread();

private:
    void AbortNotInLoopThread();

    bool looping_;  // atomic
    const pid_t tid_;
};

}   // namespace muzi

#endif  // MUZI_NET_EVENTLOOP_H_
