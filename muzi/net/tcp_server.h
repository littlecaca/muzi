#ifndef MUZI_NET_TCP_SERVER_H_
#define MUZI_NET_TCP_SERVER_H_

#include <map>
#include <memory>
#include <string>

#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "tcp_connection.h"

namespace muzi
{
class TcpServer : noncopyable
{
public:
    typedef std::function<void(EventLoop *)> ThreadInitCallback;

    TcpServer(EventLoop *loop, const InetAddress &listen_addr, 
        const std::string &name = "TcpServer", bool reuse_port = false);

    ~TcpServer();

    /// @attention Thread safe, and can be called multiple times.
    void Start();

    /// @brief Set connection callback.
    /// @attention Not thread safe.
    void SetConnectionCallback(const ConnectionCallback &cb)
    {
        connection_callback_ = cb;
    }

    void SetMessageCallback(const MessageCallback &cb)
    {
        message_callback_ = cb;
    }

private:
    /// @attention In loop.
    void NewConnection(int sock_fd, const InetAddress &peer_addr);

    /// @attention May not in loop, so it will call the "in loop" version. 
    void RemoveConnection(const TcpConnectionPtr &conn);

    /// @attention In loop. Ensure all opetions that access the data members
    /// be in loop.
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    const std::string name_;
    EventLoop *loop_;
    std::unique_ptr<Acceptor> acceptor_;    // How about just use "Acceptor accpetor_"?
    std::atomic_bool started_;
    int conn_sequence_;

    ConnectionMap connections_;

    // Callbacks that can be set by the users.
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ThreadInitCallback thread_init_callback_;
};

}   // namespace muzi

#endif  // MUZI_NET_TCP_SERVER_H_
