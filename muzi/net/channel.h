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
// Channel dose not own file descriptor.
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

    void SetReadCallback(EventCallback cb)
    {
        read_callback_ = std::move(cb);
    }

    void SetWriteCallback(EventCallback cb)
    {
        write_callback_ = std::move(cb);
    }

    void SetErrorCallback(EventCallback cb)
    {
        error_callback_ = std::move(cb);
    }

    int Getfd() const { return fd_; }

    int GetEvents() const { return events_; }

    // Get the index of the correponding poller's fds
    int GetIndex() const { return index_; }

    EventLoop *GetOwnerLoop() const { return loop_; }

    void SetIndex(int index) { index_ = index; }

    // Set active(received) events
    void SetREvents(int revts) { revents_ = revts; }

    bool IsNoneEvent() const { return events_ == kNoneEvent; }

    void EnableReading() { events_ |= kReadEvent; Update(); }

    void EnableWritting() { events_ |= kWriteEvent; Update(); }

    void DisableWritting() { events_ &= ~kWriteEvent; Update(); }

    void DisableAll() { events_ = kNoneEvent; Update(); }

    // Remove this channel from its EventLoop
    void Remove() { loop_->RemoveChannel(this); }

private:
    void Update();

private:
    EventLoop *loop_;
    const int fd_;
    int events_;    // allowed targeted events
    int revents_;   // received active events
    int index_;     // index in the poller's fds

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;

    static const int kNoneEvent = 0;
    static const int kReadEvent = POLLIN | POLLPRI;
    static const int kWriteEvent = POLLOUT;
};

}   // namespace muzi

#endif  // MUZI_NET_CHANNEL_H_
