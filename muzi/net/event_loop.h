#ifndef MUZI_NET_EVENTLOOP_H_
#define MUZI_NET_EVENTLOOP_H_

#include <atomic>
#include <memory>
#include <pthread.h>
#include <vector>

#include "current_thread.h"
#include "noncopyable.h"

namespace muzi
{
// Forward declaration
class Poller;
class Channel;

// Implementing "One loop per thread"
class EventLoop : noncopyable
{
public:
    EventLoop();
    ~EventLoop();

    void Loop();

    void Quit();

    void AssertInLoopThread()
    {
        AbortNotInLoopThread();
    }

    bool IsInLoopThread() const
    {
        return tid_ == current_thread::tid();
    }

    void UpdateChannel(Channel *channel);

    static EventLoop *GetLoopOfCurrentThread();

private:
    void AbortNotInLoopThread();
    // Make the poller stop waiting
    void WakeUp();
    void HandleWakeupRead();

private:
    const pid_t tid_;

    bool looping_;
    std::atomic_bool quit_;     // atomic
    uint64_t iteration_;
    int wakeup_fd_;
    std::unique_ptr<Channel> wakeup_channel_;

    typedef std::vector<Channel *> ChannelList;

    std::unique_ptr<Poller> poller_;
    ChannelList active_channels_;
};

}   // namespace muzi

#endif  // MUZI_NET_EVENTLOOP_H_
