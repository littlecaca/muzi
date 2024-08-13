#ifndef MUZI_NET_TCP_CONNECTION_H_
#define MUZI_NET_TCP_CONNECTION_H_

#include <atomic>
#include <memory>

#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "socket.h"


namespace muzi
{
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// Callbacks related to TcpConnection
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> ErrorCallback;


class TcpConnection : std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(std::string name,
                  EventLoop *loop,
                  int sock_fd,
                  const InetAddress &peer_addr);

    ~TcpConnection();

    void SetConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }
    void SetErrorCallback(const ErrorCallback &cb) { error_callback_ = cb; }

    void ForceClose()
    {
        HandleClose();
    }

    /// @brief Should only be called once.
    /// After set all the callbacks, the connection is ready to deal with events.
    void EstablishConnection();
    
    /// @brief Called when the connection be removed from the TcpServer's map,
    /// should only be called once.
    /// Because it is must in loop, we can not put it in the dtor.
    void DestroyConnection();

    const std::string GetName() const { return name_; }

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

    void HandleRead();
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

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    ErrorCallback error_callback_;
};

}   // namespace muzi

#endif  // MUZI_NET_TCP_CONNECTION_H_
