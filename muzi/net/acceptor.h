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
        const AddressPtr &addr)> NewConnectionCallBack;

    typedef std::function<void(const Socket &listen_sock)> StartListenCallback;
    
    Acceptor(EventLoop *loop, const AddressPtr &listen_addr, 
        bool reuse_port = false);

    /// @attention In loop.
    ~Acceptor();

    void SetNewConnectionCallBack(NewConnectionCallBack cb)
    {
        cb_ = std::move(cb);
    }

    void SetStartListenCallback(StartListenCallback cb)
    {
        start_listen_cb_ = std::move(cb);
    }

    bool IsListening() const { return listening_; }

    /// @attention In loop.
    void Listen();

    /// @brief 

    const AddressPtr &GetLocalAddr() const { return local_addr_; }

private:
    void HandleRead();
    void StopInLoop();

private:
    EventLoop *loop_;
    Socket accept_socket_;
    NewConnectionCallBack cb_;
    StartListenCallback start_listen_cb_;

    AddressPtr local_addr_;

    Channel chanel_;
    bool listening_;
    // To take up a file descriptor, when system
    // fd is used up, we can close it and accept the
    // new connection and close it too.
    int dummy_fd_;
};

}   // namespace muzi

#endif // MUZI_NET_ACCEPTOR_H_
