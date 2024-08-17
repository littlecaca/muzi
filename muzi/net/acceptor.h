#ifndef MUZI_NET_ACCEPTOR_H_
#define MUZI_NET_ACCEPTOR_H_

#include <functional>

#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "socket.h"

namespace muzi
{
/// @brief It is used by TcpServer to accept(2) new connection.
class Acceptor : noncopyable
{
public:
    typedef std::function<void(int sock_fd, 
        const InetAddress &addr)> NewConnectionCallBack;
    
    Acceptor(EventLoop *loop, const InetAddress &listen_addr, 
        bool reuse_port = false);

    /// @attention In loop.
    ~Acceptor();

    void SetNewConnectionCallBack(const NewConnectionCallBack &cb)
    {
        cb_ = cb;
    }

    bool IsListening() const { return listening_; }

    /// @attention In loop.
    void Listen();

    const InetAddress &GetLocalAddr() const { return local_addr_; }

private:
    void HandleRead();

private:
    EventLoop *loop_;
    Socket accept_socket_;
    NewConnectionCallBack cb_;

    InetAddress local_addr_;

    Channel chanel_;
    bool listening_;
    // To take up a file descriptor, when system
    // fd is used up, we can close it and accept the
    // new connection and close it too.
    int dummy_fd_;
};

}   // namespace muzi

#endif // MUZI_NET_ACCEPTOR_H_
