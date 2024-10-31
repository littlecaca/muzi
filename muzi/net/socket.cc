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

void Socket::BindAddress(const Address &addr)
{
    if (!socket::BindAddress(sock_fd_, addr.GetAddr(), addr.GetAddrSize()))
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

int Socket::Connect(const Address &addr)
{
    int cur_time = 0;
    int ret;
    while ((ret = socket::Connect(sock_fd_, addr.GetAddr(), addr.GetAddrSize())) < 0)
    {
        if (cur_time < kMaxConnectWaitSecond)
        {
            ::sleep(1);
            ++cur_time;
        }
        else
        {
            LOG_ERROR << "socket::Connect() fails";
            return ret;
        }
    }
    return 0;
}

int Socket::Accept(Address &client_addr)
{
    return socket::Accept(sock_fd_, client_addr.GetAddr());
}

ssize_t Socket::Send(const char *buffer, size_t len)
{
    ssize_t ret;
    while (true)
    {
        ret = socket::Write(sock_fd_, buffer, len);
        if (ret < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;   // retry

            LOG_ERROR << "socket::Write() fails";
        }
        break;
    }
    return ret;
}

ssize_t Socket::Recv(char *buffer, size_t len)
{
    return socket::Read(sock_fd_, buffer, len);
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
