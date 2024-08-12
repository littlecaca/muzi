#include "channel.h"

#include "logger.h"

namespace muzi
{
void Channel::HandleEvent()
{
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
}

void Channel::Remove()
{
    if (!IsNoneEvent())
    {
        DisableAll();
    }
    loop_->RemoveChannel(this);
}

void Channel::Update()
{
    loop_->UpdateChannel(this);
}
}   // namespace muzi
