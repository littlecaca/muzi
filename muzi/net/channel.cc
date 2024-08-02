#include "channel.h"

#include "logger.h"

namespace muzi
{
void Channel::HandleEvent()
{
    if (revents_ & POLLNVAL)
    {
        LOG_WARN << "Channel::handle_event() POLLNVAL";
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
}

void Channel::Update()
{
    // loop_->UpdateChannel(this);
}



}   // namespace muzi
