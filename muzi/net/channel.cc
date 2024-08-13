#include "channel.h"

#include "logger.h"

namespace muzi
{
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop), 
      fd_(fd), 
      events_(0), 
      revents_(0), 
      index_(-1),
      in_using_(false),
      handling_event_(false),
      tied_(false)
{
}

Channel::~Channel()
{
    // race condition!
    assert(!in_using_);
    assert(!handling_event_);
}

void Channel::HandleEvent()
{
    if (tied_)
    {
        std::shared_ptr<void> tied = tied_object_.lock();
        if (tied)
        {
            HandleEvent();
        }
    }
    else
    {
        HandleEvent();
    }
}

void Channel::HandleEventWithGuard()
{
    handling_event_ = true;
    
    if (revents_ & POLLNVAL)
    {
        LOG_WARN << "Channel::HandleEvent() POLLNVAL";
    }

    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (error_callback_) error_callback_();
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (read_callback_) read_callback_();
    }

    if (revents_ & POLLOUT)
    {
        if (write_callback_) write_callback_();
    }

    if (revents_ & POLLHUP && !(revents_ & POLLIN))
    {
        LOG_WARN << "Channel::HandleEvent() POLLHUP";
        if (close_callback_) close_callback_();
    }

    handling_event_ = false;
}

void Channel::Remove()
{
    assert(IsNoneEvent());
    loop_->RemoveChannel(this);
    in_using_ = false;
}

void Channel::Update()
{
    in_using_ = true;
    loop_->UpdateChannel(this);
}
}   // namespace muzi
