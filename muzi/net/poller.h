#ifndef MUZI_NET_POLLER_H_
#define MUZI_NET_POLLER_H_

#include <unordered_map>
#include <vector>

#include <sys/poll.h>

#include "channel.h"
#include "event_loop.h"
#include "noncopyable.h"
#include "timestamp.h"

namespace muzi
{
// An Abstract class for select/poll/epoll which
// are all IO  multiplexing tools.
class Poller : noncopyable
{
public:
    typedef std::vector<Channel *> ChannelList;

    Poller(EventLoop *loop) : ower_loop_(loop) {}

    virtual ~Poller() {}

    // Must be called in loop thread
    virtual Timestamp Poll(int timeout_ms, ChannelList *active_channels) = 0;

    // Must be called in loop thread
    virtual void UpdateChannel(Channel *channel) = 0;

    // Must be called in loop thread
    virtual void RemoveChannel(Channel *channel) = 0;

    virtual bool HasChannel(Channel *channel) const;

    static Poller *NewDefaultPoller(EventLoop *loop);

    void AssertInLoopThread() const { ower_loop_->AssertInLoopThread(); }

protected:
    typedef std::unordered_map<int, Channel *> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop *ower_loop_;
};

};  // MUZI_NET_POLLER_H_

#endif  // MUZI_NET_POLLER_H_
