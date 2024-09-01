#ifndef MUZI_NET_EPOLL_POLLER_H_
#define MUZI_NET_EPOLL_POLLER_H_

#include <sys/epoll.h>
#include <vector>

#include "event_loop.h"
#include "poller.h"


namespace muzi
{
// An encapsulation for epoll(7) and also a implemention of poller
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);

    ~EpollPoller();

    Timestamp Poll(int timeout_ms, ChannelList *active_channels) override;

    void UpdateChannel(Channel *channel) override;

    void RemoveChannel(Channel *channel) override;

private:
    typedef std::vector<struct epoll_event> EventList;

    const int kInitialEventListSize = 16;

    void FillActiveChannels(int event_num, ChannelList *active_channels) const;
    
    void Update(int operation, Channel *channel);

    int epfd_;
    EventList ep_events_;
    struct epoll_event event_;
};
}   // namespace muzi

#endif  // MUZI_NET_EPOLL_POLLER_H_
