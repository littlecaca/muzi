#include "event_loop.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <sys/signal.h>
#include <sys/poll.h>

#include "timer_queue.h"
#include "poller.h"
#include "logger.h"

namespace muzi
{
namespace
{
thread_local EventLoop *t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10000;

int CreateEventFd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd == -1)
    {
        LOG_SYSFAT << "CreateEventFd() fails";
    }
    return fd;
}

/// If the client has closed the connection gracefully (using close() or shutdown()), 
/// and you try to write to the socket, the server will receive a SIGPIPE signal by default.
/// The write operation will fail, and write() will return -1 with errno set to EPIPE. 
/// If the SIGPIPE signal is not handled or ignored, the process will terminate.
/// So we need to make the program ingnore the SIGPIPE signal.
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSigPipe ignore;

}   // internal linkage

EventLoop::EventLoop() 
    : tid_(current_thread::tid()),
      looping_(false),
      quit_(false),
      iteration_(0),
      wakeup_fd_(CreateEventFd()),
      calling_functors_(false),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      poller_(Poller::NewDefaultPoller(this)),
      timer_queue_(new TimerQueue(this))
{
    LOG_TRACE << "Created EventLoop " << this << " in thread " << tid_;
    if (t_loop_in_this_thread)
    {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << tid_;
    }
    else
    {
        t_loop_in_this_thread = this;
    }

    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleWakeupRead, this));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeup_fd_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::Loop()
{
    assert(!looping_);
    assert(!quit_);
    AssertInLoopThread();
    looping_ = true;

    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        active_channels_.clear();
        Timestamp time = poller_->Poll(kPollTimeMs, &active_channels_);
        ++iteration_;

        for (Channel *channel : active_channels_)
        {
            channel->HandleEvent(time);
        }

        ExecuteFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::Quit()
{
    quit_ = true;
    if (!IsInLoopThread())
    {
        WakeUp();
    }
}

void EventLoop::AbortNotInLoopThread()
{
    if (!IsInLoopThread())
    {
        LOG_FATAL << "This loop belongs to thread " << tid_ 
                  << " instead of current thread " << current_thread::tid();
    }
}

void EventLoop::UpdateChannel(Channel *channel)
{
    assert(channel->GetOwnerLoop() == this);
    poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel *channel)
{
    assert(channel->GetOwnerLoop() == this);
    poller_->RemoveChannel(channel);
}

void EventLoop::WakeUp()
{
    eventfd_t val = 1;
    int n = ::eventfd_write(wakeup_fd_, val);
    if (n != 0)
    {
        LOG_ERROR << "eventfd_write fails";
    }
}

void EventLoop::HandleWakeupRead()
{
    eventfd_t val;
    eventfd_read(wakeup_fd_, &val);
}

void EventLoop::QueueInLoop(const Functor &cb)
{
    {
        MutexLockGuard guard(lock_);
        functors_.push_back(cb);
    }
    
    if (!IsInLoopThread() || calling_functors_)
    {
        WakeUp();
    }
}

// Guaranteed in loop thread
// Only be called in Loop()
void EventLoop::ExecuteFunctors()
{
    FunctorList cur_functors;
    calling_functors_ = true;

    {
        MutexLockGuard guard(lock_);
        cur_functors.swap(functors_);
    }

    for (const Functor &func : cur_functors)
    {
        func();

    }
    calling_functors_ = false;
}

EventLoop *EventLoop::GetLoopOfCurrentThread()
{
    return t_loop_in_this_thread;
}

TimerId EventLoop::RunAt(Timestamp when, const TimerCallback &cb)
{
    return timer_queue_->AddTimer(cb, when, 0.0);
}

TimerId EventLoop::RunAfter(double delay, const TimerCallback &cb)
{
    return timer_queue_->AddTimer(cb, Timestamp().AddTime(delay), 0.0);
}

TimerId EventLoop::RunEvery(Timestamp when, const TimerCallback &cb, double interval)
{
    return timer_queue_->AddTimer(cb, when, interval);
}

void EventLoop::RunInLoop(const Functor &cb)
{
    if (IsInLoopThread())
    {
        cb();
    }
    else
    {
        QueueInLoop(cb);
    }
}

}   // namespace muzi
