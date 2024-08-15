#ifndef MUZI_NET_TCP_CONNECTION_H_
#define MUZI_NET_TCP_CONNECTION_H_

#include <atomic>
#include <memory>

#include "buffer.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "socket.h"
#include "timestamp.h"


namespace muzi
{
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// Callbacks related to TcpConnection
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &, Buffer *buf, Timestamp time)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> ErrorCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;


class TcpConnection : std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(std::string name,
                  EventLoop *loop,
                  int sock_fd,
                  const InetAddress &peer_addr);

    ~TcpConnection();

    /// @attention Not must in loop.
    void SetConnectionCallback(ConnectionCallback cb) { connection_callback_ = std::move(cb); }
    /// @attention Not must in loop.
    void SetMessageCallback(MessageCallback cb) { message_callback_ = std::move(cb); }
    /// @attention Not must in loop.
    void SetCloseCallback(CloseCallback cb) { close_callback_ = std::move(cb); }
    /// @attention Not must in loop.
    void SetErrorCallback(ErrorCallback cb) { error_callback_ = std::move(cb); }

    void ForceClose()
    {
        HandleClose();
    }

    /// @brief Should only be called once.
    /// After set all the callbacks, the connection is ready to deal with events.
    /// @attention Because this operation must in loop, so we can not put it in
    /// ctor.
    void EstablishConnection();
    
    /// @brief Called when the connection be removed from the TcpServer's map,
    /// should only be called once.
    /// Because it is must in loop, we can not put it in the dtor.
    void DestroyConnection();

    const std::string GetName() const { return name_; }

    EventLoop *GetLoop() const { return loop_; }

private:
    enum ConnectionState
    {
        kConnecting,
        kConnected,
        kDisConnected,
    };

    void SetState(ConnectionState state)
    {
        state_ = state;
    }

    void HandleRead(Timestamp received_time);
    void HandleWrite();
    void HandleClose();
    void HandleError();

private:
    std::string name_;
    EventLoop *loop_;
    std::atomic<ConnectionState> state_;
    
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress peer_addr_;

    Buffer input_buffer_;
    
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    ErrorCallback error_callback_;
};

}   // namespace muzi

#endif  // MUZI_NET_TCP_CONNECTION_H_
