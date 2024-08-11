#include "inet_address.h"

#include <assert.h>

#include <endian.h>
#include <netdb.h>

#include "logger.h"

namespace muzi
{
// Because we use union type in InetAddress class, the member offset
// of struct sockaddr_in and sockaddr_in6 must keep consistent.
static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6), 
    "InetAddress has the same size with struct sockaddr_in6");
static_assert(offsetof(struct sockaddr_in, sin_family) == offsetof(struct sockaddr_in6, sin6_family), 
    "sin_family and sin6_family should have the same offset");
static_assert(offsetof(struct sockaddr_in, sin_port) == offsetof(struct sockaddr_in6, sin6_port),
    "sin_port and sin6_port should have the same offset");

namespace
{
thread_local char t_resolve_buffer[64 * 1024];
}   // interval linkage

InetAddress::InetAddress(uint16_t port, bool loop_back_only, bool ipv6)
{
    if (ipv6)
    {
        memset(&addr6_, 0, sizeof addr6_);
        addr6_.sin6_family = AF_INET6;
        addr6_.sin6_port = static_cast<in_port_t>(endian::HostToNet16(port));
        struct in6_addr ip = loop_back_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
    }
    else
    {
        memset(&addr_, 0, sizeof addr_);
        addr_.sin_family = AF_INET;
        addr_.sin_port = static_cast<in_port_t>(endian::HostToNet16(port));
        uint32_t ip = endian::HostToNet32(loop_back_only ? INADDR_LOOPBACK : INADDR_ANY);
        addr_.sin_addr.s_addr = static_cast<in_addr_t>(ip);
    }
}

InetAddress::InetAddress(StringProxy ip, uint16_t port, bool ipv6)
{
    if (ipv6)
    {
        socket::FromIpPort(ip.data(), port, &addr6_);
    }
    else
    {
        socket::FromIpPort(ip.data(), port, &addr_);
    }
}

std::string InetAddress::GetIpStr() const
{
    char buf[64];
    socket::ToIp(buf, sizeof buf, socket::SockAddrCast(&addr6_));
    return buf;
}

std::string InetAddress::GetIpPortStr() const
{
    char buf[64];
    socket::ToIpPort(buf, sizeof buf, socket::SockAddrCast(&addr6_));
    return buf;
}

uint32_t InetAddress::GetIpv4NetEndian() const
{
    if (!IsIpv4())
    {
        LOG_ERROR << "Not a ipv4";
        return -1;
    }
    return static_cast<uint32_t>(addr_.sin_addr.s_addr);
}

void InetAddress::SetScopeId(uint32_t scoped_id)
{
    if (GetFamily() == AF_INET6)
    {
        addr6_.sin6_scope_id = scoped_id;
    }
}

bool InetAddress::ResolveHost(StringProxy hostname, InetAddress *addr)
{
    if (addr == nullptr)
    {
        LOG_ERROR << "addr is nullptr";
        return false;
    }

    hostent hent;
    hostent *res = NULL;
    int herrno = 0;
    int ret = ::gethostbyname_r(hostname.data(), &hent, 
        t_resolve_buffer, sizeof t_resolve_buffer, &res, &herrno);
    
    if (ret != 0 || res == nullptr)
    {
        LOG_SYSERR << "::gethostbyname_r() fails";
        return false;
    }

    if (hent.h_addrtype != AF_INET)
    {
        LOG_ERROR << "::gethostbyname_r() only deal with ipv4";
        return false;
    }

    addr->addr_.sin_addr = *reinterpret_cast<in_addr *>(res->h_addr);

    return true;
}

} // namespace muzi
