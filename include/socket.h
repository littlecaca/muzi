#ifndef MUZI_NET_SOCKET_H_
#define MUZI_NET_SOCKET_H_

#include <unistd.h>

#include "string_proxy.h"
#include "inet_address.h"
#include "noncopyable.h"

namespace muzi
{
/// @brief A wrapper of socket file descriptor.
class Socket : noncopyable
{
public:
    explicit Socket(int sock_fd) : sock_fd_(sock_fd)
    {}

    ~Socket()
    {
        socket::Close(sock_fd_);
    }

    int GetFd() const { return sock_fd_; }

    std::string GetTcpInfo();

    /// @brief A wrapper of ::bind().
    void BindAddress(const Address &addr);

    /// @brief A wrapper of ::listen().
    void Listen();

    /// @brief A wrapper of ::connect().
    int Connect(const Address &addr);

    /// @brief A wrapper of ::accept().
    /// @attention The new socket will be set NONBLOCK and CLOEXEC
    /// @return sock_fd for new connection if success, -1 otherwise.
    int Accept(Address &client_addr);

    ssize_t Send(const char *buffer, size_t len);
    ssize_t Recv(char *buffer, size_t len);

    /// @brief A wrapper of ::shutdown(), half-close the output stream.
    void ShutDownOnWrite();

    /// @brief Set TCP options of TCP_NODELAY.
    void SetTcpNoDelay(bool on);
    
    /// @brief Set socket options of SO_REUSEADDR.
    void SetReuseAddr(bool on);

    /// @brief Set socket options of SO_REUSEPORT.
    void SetReusePort(bool on);

    /// @brief Set socket options of SO_KEEPALIVE.
    void SetKeepAlive(bool on);

private:
    int sock_fd_;

    static constexpr int kMaxConnectWaitSecond = 5;   // 5s
};

}   // namespace muzi

#endif // MUZI_NET_SOCKET_H_
