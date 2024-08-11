#ifndef MUZI_NET_EVENTLOOP_H_
#define MUZI_NET_EVENTLOOP_H_

#include <atomic>
#include <memory>
#include <pthread.h>
#include <vector>

#include "current_thread.h"
#include "mutex.h"
#include "noncopyable.h"
#include "timer.h"

namespace muzi
{
// Forward declaration
class Poller;
class Channel;
class TimerQueue;

class EventLoop : noncopyable
{
    friend class Channel;
public:
    typedef std::function<void()> Functor;
    
    EventLoop();
    ~EventLoop();

    /// @attention Must be called in its own thread
    void Loop();

    /// @attention Can be called in other threads
    void Quit();

    void AssertInLoopThread()
    {
        AbortNotInLoopThread();
    }

    bool IsInLoopThread() const
    {
        return tid_ == current_thread::tid();
    }

    static EventLoop *GetLoopOfCurrentThread();

    /// @attention Can be called in other threads
    TimerId RunAt(Timestamp when, const TimerCallback &cb);

    /// @attention Can be called in other threads
    TimerId RunAfter(double delay, const TimerCallback &cb);

    /// @attention Can be called in other threads
    TimerId RunEvery(Timestamp when, const TimerCallback &cb, double interval);

    /// @brief For syncronization
    /// @attention Can be called in other threads
    void RunInLoop(const Functor &cb);

private:
    // Used by friend class Channel
    
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);

private:
    void AbortNotInLoopThread();
    /// @brief Make the poller stop waiting
    void WakeUp();
    void HandleWakeupRead();
    void QueueInLoop(const Functor &cb);
    void ExecuteFunctors();

private:
    typedef std::vector<Channel *> ChannelList;
    typedef std::vector<Functor> FunctorList;

    const pid_t tid_;
    bool looping_;
    std::atomic_bool quit_;     // atomic
    uint64_t iteration_;
    int wakeup_fd_;
    bool calling_functors_;
    
    std::unique_ptr<Channel> wakeup_channel_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;

    MutexLock lock_;
    
    // Used by RunInLoop and QueueInLoop to store tasks
    FunctorList functors_ GUARDED_BY(lock_);
    ChannelList active_channels_;
};

}   // namespace muzi

#endif  // MUZI_NET_EVENTLOOP_H_
