#ifndef MUZI_NET_POLLER_H_
#define MUZI_NET_POLLER_H_

#include <sys/poll.h>
#include <map>
#include <vector>

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

    ~Poller() {}

    // Must be called in loop thread
    virtual TimeStamp Poll(int timeout_ms, ChannelList *active_channels) = 0;

    // Must be called in loop thread
    virtual void UpdateChannel(Channel *channel) = 0;

    // Must be called in loop thread
    virtual void RemoveChannel(Channel *channel) = 0;

    virtual bool HasChannel(Channel *channel) const;

    virtual Poller *NewPoller(EventLoop *loop) const = 0;

    void AssertInLoopThread() const { ower_loop_->AssertInLoopThread(); }

protected:
    typedef std::map<int, Channel *> ChannelMap;
    ChannelMap channels_;

private:
    EventLoop *ower_loop_;
};

};  // MUZI_NET_POLLER_H_

#endif  // MUZI_NET_POLLER_H_
