#include "socket.h"

#include "logger.h"

namespace muzi
{
std::string Socket::GetTcpInfo()
{
    char buf[512];
    if (socket::GetTcpInfoString(sock_fd_, buf, sizeof buf))
    {
        return buf;
    }
    else
    {
        LOG_ERROR << "socket::GetTcpInfoString() fails";
        return "Unknown Info";
    }
}

void Socket::BindAddress(const InetAddress &addr)
{
    if (!socket::BindAddress(sock_fd_, addr.GetAddr()))
    {
        LOG_FATAL << "socket::BindAddress() fails";
    }
}

void Socket::Listen()
{
    if (!socket::Listen(sock_fd_))
    {
        LOG_FATAL << "socket::Listen() fails";
    }
}

int Socket::Accept(InetAddress &client_addr)
{
    return socket::Accept(sock_fd_, client_addr.GetAddr());
}

void Socket::ShutDownOnWrite()
{
    socket::ShutDownOnWrite(sock_fd_);
}

void Socket::SetTcpNoDelay(bool on)
{
    socket::SetSockOpt(sock_fd_, SOL_TCP, TCP_NODELAY, on);
}

void Socket::SetReuseAddr(bool on)
{
    socket::SetSockOpt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, on);
}

void Socket::SetReusePort(bool on)
{
    socket::SetSockOpt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, on);
}

void Socket::SetKeepAlive(bool on)
{
    socket::SetSockOpt(sock_fd_, SOL_SOCKET, SO_KEEPALIVE, on);
}

}   // namespace muzi
