#include "socket_ops.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "endian_transform.h"
#include "logger.h"

namespace muzi
{
namespace socket
{
void AbortNotInterNet(const sockaddr *addr)
{
    if (addr->sa_family != AF_INET || addr->sa_family != AF_INET6)
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

}   // namespace socket
}   // namespace muzi
