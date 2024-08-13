#include "socket_ops.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include "endian_transform.h"
#include "logger.h"

namespace muzi
{
namespace socket
{
void AbortNotInterNet(const sockaddr *addr)
{
    if (addr->sa_family != AF_INET && addr->sa_family != AF_INET6)
    {
        LOG_FATAL << "Only allow AF_INET and AF_INET6 address family";
    }
}

void FromIpPort(const char *ip, uint16_t port, sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = endian::HostToNet16(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR << "::inet_pton() fails";
    }
}

void FromIpPort(const char *ip, uint16_t port, sockaddr_in6 *addr6)
{
    addr6->sin6_family = AF_INET6;
    addr6->sin6_port = endian::HostToNet16(port);
    if (::inet_pton(AF_INET6, ip, &addr6->sin6_addr) <= 0)
    {
        LOG_SYSERR << "::inet_pton() fails";
    }
}

void ToIpPort(char *buf, int buf_len, const sockaddr *addr)
{
    AbortNotInterNet(addr);

    ToIp(buf, buf_len, addr);
    int len = strlen(buf);
    if (buf_len - len >= 6)
    {
        buf[len] = ':';
        ToPort(buf + len + 1, buf_len - len - 1, addr);
    }
}

void ToIp(char *buf, int buf_len, const sockaddr *addr)
{
    AbortNotInterNet(addr);

    if (addr->sa_family == AF_INET)
    {
        const struct sockaddr_in *addr_in = SockAddrInCast(addr);
        if (::inet_ntop(AF_INET, &addr_in->sin_addr, buf, 
            static_cast<socklen_t>(buf_len)) == nullptr)
        {
            LOG_SYSERR << "::inet_ntop() fails";
        }
    }
    else
    {
        const struct sockaddr_in6 *addr_in6 = SockAddrIn6Cast(addr);
        if (::inet_ntop(AF_INET6, &addr_in6->sin6_addr, buf, 
            static_cast<socklen_t>(buf_len)) == nullptr)
        {
            LOG_SYSERR << "::inet_ntop() fails";
        }
    }
}

void ToPort(char *buf, int buf_len, const sockaddr *addr)
{
    AbortNotInterNet(addr);

    uint16_t port_net;
    if (addr->sa_family == AF_INET)
    {
        port_net = SockAddrInCast(addr)->sin_port;
    }
    else
    {
        port_net = SockAddrIn6Cast(addr)->sin6_port;
    }
    uint16_t port = endian::NetToHost16(port_net);
    snprintf(buf, buf_len, "%hu", port);
}

void Close(int fd)
{
    if (::close(fd) < 0)
    {
        LOG_SYSERR << "::close() fails";
    }
}

bool GetTcpInfo(int sock_fd, tcp_info *info)
{
    socklen_t len = sizeof(*info);
    memset(info, 0, len);
    if (::getsockopt(sock_fd, SOL_TCP, TCP_INFO, info, &len))
    {
        LOG_SYSERR << "::getsockopt() fails";
        return false;
    }
    return true;
}

bool GetTcpInfoString(int sock_fd, char *buf, int len)
{
    tcp_info info;
    if (!GetTcpInfo(sock_fd, &info))
        return false;
    snprintf(buf, len, "unrecovered=%u "
        "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
        "lost=%u retrans=%u rtt=%u rttvar=%u "
        "snd_ssthresh=%u snd_cwnd=%u total_retrans=%u",
        info.tcpi_retransmits,
        info.tcpi_rto,
        info.tcpi_ato,
        info.tcpi_snd_mss,
        info.tcpi_rcv_mss,
        info.tcpi_lost,
        info.tcpi_retrans,
        info.tcpi_rtt,
        info.tcpi_rttvar,
        info.tcpi_snd_ssthresh,
        info.tcpi_snd_cwnd,
        info.tcpi_total_retrans);
    
    return true;
}

bool BindAddress(int sock_fd, const sockaddr *addr)
{
    if (::bind(sock_fd, addr, static_cast<socklen_t>(sizeof (sockaddr_in6))) < 0)
    {
        LOG_SYSERR << "::bind() fails";
        return false;
    }
    return true;
}

bool Listen(int sock_fd)
{
    if (::listen(sock_fd, SOMAXCONN) < 0)
    {
        LOG_SYSERR << "::listen() fails";
        return false;
    }
    return true;
}

bool Connect(int sock_fd, const sockaddr *addr)
{
    if (::connect(sock_fd, addr, sizeof (sockaddr_in6)) < 0)
    {
        LOG_SYSERR << "::connect() fails";
        return false;
    }
    return true;
}

int Accept(int sock_fd, sockaddr *addr)
{
    socklen_t addr_len = static_cast<socklen_t>(sizeof *addr);
    int sock = ::accept4(sock_fd, addr, &addr_len, O_NONBLOCK | O_CLOEXEC);

    // The special problem of accept()ing when you can't
    // Refer to this:
    // http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_accept_ing_wh
    // 
    // Here we allow ENFILE and EMFILE error to pass to guide the upper users
    // to cope with it further.
    if (sock < 0)
    {
        switch (errno)
        {
        case EMFILE:    // too may open files
        case EAGAIN:    // try again
        case EINTR:     // Interrupted system call
        case ECONNABORTED:  // Software caused connection abort
            break;
        default:
            LOG_FATAL << "Unexpeted error ::accept4() fails";
            break;
        }
    }

    return sock;
}

bool ShutDownOnWrite(int sock_fd)
{
    if (::shutdown(sock_fd, SHUT_WR) < 0)
    {
        LOG_SYSERR << "::shutdown() fails";
        return false;
    }
    return true;
}

bool SetSockOpt(int sock_fd, int level, int opt, bool on)
{
    int option = on;
    if (::setsockopt(sock_fd, level, opt, 
        &option, static_cast<socklen_t>(sizeof option)) < 0)
    {
        LOG_SYSERR << "::setsockopt() fails";
        return false;
    }
    return true;
}

int CreateBlockingSockOrDie()
{
    int sock = ::socket(PF_INET, SOCK_STREAM | O_CLOEXEC, 0);
    if (sock < 0)
    {
        LOG_SYSFAT << "::socket() fails";
    }
    return sock;
}

int CreateNonBlockingSockOrDie()
{
    int sock = ::socket(PF_INET, SOCK_STREAM | O_CLOEXEC | O_NONBLOCK, 0);
    if (sock < 0)
    {
        LOG_SYSFAT << "::socket() fails";
    }
    return sock;
}

sockaddr_in6 GetLocalAddr(int sock_fd)
{
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof addr);
    socklen_t addr_len = static_cast<socklen_t>(sizeof(sockaddr_in6));
    if (::getsockname(sock_fd, SockAddrCast(&addr), &addr_len) < 0)
    {
        LOG_SYSERR << "::getsockname() fails";
    }
    return addr;
}

}   // namespace socket
}   // namespace muzi
