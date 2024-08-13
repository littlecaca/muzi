#ifndef MUZI_NET_CHANNEL_H_
#define MUZI_NET_CHANNEL_H_

#include <functional>

#include <sys/poll.h>

#include "event_loop.h"
#include "noncopyable.h"

namespace muzi
{

/// @brief Channel class is responsible for dipathing IO events to handle funcs.
/// It is not thread safe.
/// Channel dose not own the file descriptor.
class Channel : noncopyable
{
public:
    typedef std::function<void()> EventCallback;

    Channel(EventLoop *loop, int fd);

    /// @attention There may be a race condition in it.
    ~Channel();

    /// @attention In loop.
    void HandleEvent();

    void HandleEventWithGuard();

    /// @brief Keep the tied_object alive when handling event.
    void Tie(std::shared_ptr<void> object)
    {
        tied_object_ = object;
        tied_ = true;
    }

    void SetReadCallback(EventCallback cb)
    {
        read_callback_ = std::move(cb);
    }

    void SetWriteCallback(EventCallback cb)
    {
        write_callback_ = std::move(cb);
    }

    void SetCloseCallback(EventCallback cb)
    {
        close_callback_ = cb;
    }

    void SetErrorCallback(EventCallback cb)
    {
        error_callback_ = std::move(cb);
    }

    int Getfd() const { return fd_; }

    /// @attention Not thread safe
    int GetEvents() const { return events_; }

    /// @brief Get the index of the correponding poller's fds
    /// @attention Not thread safe
    int GetIndex() const { return index_; }

    EventLoop *GetOwnerLoop() const { return loop_; }

    /// @attention Not thread safe
    void SetIndex(int index) { index_ = index; }

    /// @brief Set active(received) events
    void SetREvents(int revts) { revents_ = revts; }

    bool IsNoneEvent() const { return events_ == kNoneEvent; }

    /// @attention In loop.
    void EnableReading() { events_ |= kReadEvent; Update(); }

    /// @attention In loop.
    void EnableWritting() { events_ |= kWriteEvent; Update(); }

    /// @attention In loop.
    void DisableWritting() { events_ &= ~kWriteEvent; Update(); }

    /// @attention In loop.
    void DisableAll() { events_ = kNoneEvent; Update(); }

    /// @brief Remove this channel from its EventLoop.
    /// If necessay, it will call DisableAll().
    /// @attention In loop.
    void Remove();

private:
    void Update();

private:
    EventLoop *loop_;
    const int fd_;
    int events_;    // allowed targeted events
    int revents_;   // received active events
    int index_;     // index in the poller's fds
    bool in_using_; 
    bool handling_event_;
    bool tied_;
    std::weak_ptr<void> tied_object_;

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;

    static const int kNoneEvent = 0;
    static const int kReadEvent = POLLIN | POLLPRI;
    static const int kWriteEvent = POLLOUT;
};

}   // namespace muzi

#endif  // MUZI_NET_CHANNEL_H_
