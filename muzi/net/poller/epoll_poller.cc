#include "epoll_poller.h"

#include <errno.h>

#include "logger.h"

namespace muzi
{
static_assert(EPOLLIN == POLLIN, "epoll should use the same flag values with poll");
static_assert(EPOLLPRI == POLLPRI, "epoll should use the same flag values with poll");
static_assert(EPOLLOUT == POLLOUT, "epoll should use the same flag values with poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll should use the same flag values with poll");
static_assert(EPOLLERR == POLLERR, "epoll should use the same flag values with poll");
static_assert(EPOLLHUP == POLLHUP, "epoll should use the same flag values with poll");

namespace 
{
// Mark fd status in epoll
enum FdStatus
{
    kNew,
    kAdded,
    kDeleted
};

const char *ToOperationStr(int operation)
{
    switch (operation)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_MOD:
        return "MOD";
    case EPOLL_CTL_DEL:
        return "DEL";
    default:
        return "UNKNOWN";
    }
}
}   // internal linkage

EpollPoller::EpollPoller(EventLoop *loop) 
    : Poller(loop),
      epfd_(::epoll_create1(EPOLL_CLOEXEC)),
      ep_events_(kInitialEventListSize)
{      
    if (epfd_ == -1)
    {
        LOG_SYSFAT << "EpollPoller::EpollPoller: epoll_create1() fails";
    }
}

TimeStamp EpollPoller::Poll(int timeout_ms, ChannelList *active_channels)
{
    AssertInLoopThread();

    int event_num = ::epoll_wait(epfd_, ep_events_.data(), 
        static_cast<int>(ep_events_.size()), timeout_ms);

    TimeStamp now;
    int saved_errno = errno;
    
    if (event_num > 0)
    {
        LOG_TRACE << event_num << "events happened";
        FillActiveChannels(event_num, active_channels);

        // Extend the event buffer if needed
        if (event_num == ep_events_.size())
        {
            ep_events_.resize(ep_events_.size() * 2);
        }
    }
    else if (event_num == 0)
    {
        LOG_TRACE << "No event happened";
    }
    else
    {
        errno = saved_errno;
        LOG_SYSERR << "EpollPoller::poll(): epoll_wait() fails";
    }

    return now;
}

void EpollPoller::UpdateChannel(Channel *channel)
{
    AssertInLoopThread();

    const int index = channel->GetIndex();
    int fd = channel->Getfd();
    auto it = channels_.find(fd);   

    LOG_TRACE << ToOperationStr(index) << " fd " << fd << " in EpollPoller " << epfd_
              << " with events " << channel->GetEvents();

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            assert(it == channels_.end());
            // Add the new channel to EpollPoller
            channels_[fd] = channel;
        }
        else
        {
            // Must ensure the channel is already in EpollPoller
            assert(it != channels_.end());
            assert(it->second == channel);
        }

        channel->SetIndex(kAdded);
        Update(EPOLL_CTL_ADD, channel);
    }
    else if (index == kAdded)
    {
        assert(it != channels_.end());
        assert(it->second == channel);
        if (channel->IsNoneEvent())
        {
            // If no event to monitor, delete it from epoll
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(kDeleted);
        }
        else
        {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
    else
    {
        LOG_SYSFAT << "Unkonwn channel status value " << index;
    }
} 

// Remove channel from epoll and EpollPoller
void EpollPoller::RemoveChannel(Channel *channel)
{
    AssertInLoopThread();

    int fd = channel->Getfd();
    LOG_TRACE << "Remove fd " << fd << "from EpollPoller " << epfd_;

    auto it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
    
    int index = channel->GetIndex();

    if (index == kAdded || index == kDeleted)
    {
        // Remember that the fd's status in epoll and EpollPoller may not syncrosize
        if (index == kAdded)
        {
            Update(EPOLL_CTL_DEL, channel);
        }

        channels_.erase(fd);
    }
    else
    {
        LOG_ERROR << "Op " << ToOperationStr(index) << " fd " << fd;
    }
}

void EpollPoller::FillActiveChannels(int event_num, ChannelList *active_channels) const
{
    assert(event_num <= ep_events_.size());
    for (int i = 0; i < event_num; ++i)
    {
        Channel *channel = static_cast<Channel *>(ep_events_[i].data.ptr);
        channel->SetREvents(ep_events_[i].events);
        active_channels->push_back(channel);
    }
}

void EpollPoller::Update(int operation, Channel *channel)
{
    memset(&event_, 0, sizeof event_);
    event_.events = static_cast<uint32_t>(channel->GetEvents());
    int fd = channel->Getfd();
    event_.data.ptr = static_cast<void *>(channel);

    if (::epoll_ctl(epfd_, operation, fd, &event_) < 0)
    {
        LOG_SYSFAT << "epoll op " << ToOperationStr(operation) 
                   << "fails, fd = " << fd << "epfd = " << epfd_;
    }
}

}   // namespace muzi
