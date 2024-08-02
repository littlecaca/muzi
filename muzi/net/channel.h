#ifndef MUZI_NET_CHANNEL_H_
#define MUZI_NET_CHANNEL_H_

#include <functional>
#include <sys/poll.h>

#include "event_loop.h"
#include "noncopyable.h"

namespace muzi
{

// Channel class is responsible for dipathing IO events to handle funcs.
// It must belong to only one thread.
class Channel : noncopyable
{
public:
    typedef std::function<void()> EventCallback;

    Channel(EventLoop *loop, int fd) 
        : loop_(loop), 
          fd_(fd), 
          events_(0), 
          revents_(0), 
          index_(-1)
    {}

    void HandleEvent();

    void SetReadCallback(const EventCallback &cb)
    {
        read_callback_ = cb;
    }

    void SetWriteCallback(const EventCallback &cb)
    {
        write_callback_ = cb;
    }

    void SetErrorCallback(const EventCallback &cb)
    {
        error_callback_ = cb;
    }

    int Getfd() const { return fd_; }

    int GetEvents() const { return events_; }

    void SetREvents(int revts) { revents_ = revts; }

    bool IsNoneEvent() const { return events_ == kNoneEvent; }

    void EnableReading() { events_ |= kReadEvent; Update(); }

    void EnableWritting() { events_ |= kWriteEvent; Update(); }

    void DisableWritting() { events_ &= ~kWriteEvent; Update(); }

    void DisableAll() { events_ = kNoneEvent; Update(); }

private:
    void Update();

private:
    EventLoop *loop_;
    const int fd_;
    int events_;    // targeted events
    int revents_;   // active events
    int index_;     // index in the poller pollfds_

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;

    static const int kNoneEvent = 0;
    static const int kReadEvent = POLLIN | POLLPRI;
    static const int kWriteEvent = POLLOUT;
};

}   // namespace muzi

#endif  // MUZI_NET_CHANNEL_H_
