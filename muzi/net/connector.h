#ifndef MUZI_NET_CONNECTOR_H_
#define MUZI_NET_CONNECTOR_H_

#include <atomic>

#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"

namespace muzi
{
class Connector : noncopyable,
                  public std::enable_shared_from_this<Connector>
{
public:
    typedef std::function<void(int sock_fd)> NewConnectionCallback;

    static constexpr size_t kInitRetryDelayMs = 500;
    static constexpr size_t kMaxRetryDelayMs = 30 * 1000;

    Connector(EventLoop *loop, const InetAddress &server_addr);

    ~Connector();

    void Start();
    void Stop();
    void ReStartInLoop();

    /// @attention Not thread safe.
    void SetNewConnectionCallback(NewConnectionCallback cb)
    {
        new_connection_callback_ = std::move(cb);
    }

private:
    enum State
    {
        kDisConnected, kConnecting, kConnected,
    };

    void StartInLoop();
    void StopInLoop();

    void HandleWrite();
    void HandleError();

    void Connect();
    void Connecting(int sock_fd);
    void Retry(int sock_fd);

    int RemoveAndResetChannel();
    void ResetChannel();

    void SetState(State stat)
    {
        state_ = stat;
    }

private:
    EventLoop *loop_;
    InetAddress server_addr_;

    std::atomic_bool connect_;
    std::unique_ptr<Channel> channel_;
    // Not revealed to users.
    State state_;
    size_t retry_delay_;
    
    NewConnectionCallback new_connection_callback_;
};
}   // namespace muzi

#endif  // MUZI_NET_CONNECTOR_H_
