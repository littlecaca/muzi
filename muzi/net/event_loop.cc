#include "event_loop.h"

#include <sys/eventfd.h>
#include <sys/poll.h>

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

}   // internal linkage

EventLoop::EventLoop() 
    : tid_(current_thread::tid()),
      looping_(false),
      quit_(false),
      iteration_(0),
      wakeup_fd_(CreateEventFd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      poller_(Poller::NewDefaultPoller(this))
{
    LOG_TRACE << "Created " << this << " in thread " << tid_;
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
        poller_->Poll(kPollTimeMs, &active_channels_);
        ++iteration_;

        for (Channel *channel : active_channels_)
        {
            channel->HandleEvent();
        }

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

void EventLoop::WakeUp()
{
    eventfd_t val = 1;
    int n = ::eventfd_write(wakeup_fd_, val);
    if (n != sizeof val)
    {
        LOG_ERROR << "eventfd_write writes " << n << "bytes rather than " << sizeof val << "bytes";
    }
}

void EventLoop::HandleWakeupRead()
{
    eventfd_t val;
    eventfd_read(wakeup_fd_, &val);
}

void EventLoop::UpdateChannel(Channel *channel)
{
    AssertInLoopThread();
    assert(channel->GetOwnerLoop() == this);

    poller_->UpdateChannel(channel);
}

EventLoop *EventLoop::GetLoopOfCurrentThread()
{
    return t_loop_in_this_thread;
}

}   // namespace muzi
