#include "poller.h"

namespace muzi
{
bool Poller::HasChannel(Channel *channel) const
{
    AssertInLoopThread();
    auto it = channels_.find(channel->Getfd());
    return it != channels_.end() && it->second == channel;
}
}   // namespace muzi
