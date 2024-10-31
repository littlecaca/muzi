#ifndef MUZI_NET_INET_ADDRESS_H_
#define MUZI_NET_INET_ADDRESS_H_

#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "address.h"
#include "endian_transform.h"
#include "socket_ops.h"
#include "string_proxy.h"

namespace muzi
{
/// @brief A simple encapsulation to struct sockaddr_in and sruct sockaddr_in6
/// It hide the concrete type of the address (ipv4 or ipv6).
class InetAddress : public Address
{
public:
    /// @brief Automaticlly set ip to local. Used to listening.
    explicit InetAddress(uint16_t port = 0, bool loop_back_only = false, bool ipv6 = false);

    InetAddress(StringProxy ip, uint16_t port, bool ipv6 = false);

    InetAddress(const struct sockaddr_in &addr)
        : addr_(addr)
    { }

    InetAddress(const struct sockaddr_in6 &addr_6)
        : addr6_(addr_6)
    { }

public:
    const struct sockaddr *GetAddr() const override { return socket::SockAddrCast(&addr6_); }
    struct sockaddr *GetAddr() override { return socket::SockAddrCast(&addr6_); }
    sa_family_t GetFamily() const override { return addr_.sin_family; }
    std::string GetAddrStr() const override { return GetIpPortStr(); }
    AddressPtr Copy() const override { return std::make_shared<InetAddress>(*this); }
    int GetProtoFamily() const override
    {
        if (GetFamily() == AF_INET) return PF_INET;
        else return PF_INET6;
    }

    size_t GetAddrSize() const override
    {
        if (GetFamily() == AF_INET)
        {
            return sizeof(sockaddr_in);
        }
        else
        {
            return sizeof(sockaddr_in6);
        }
    }

    void SetAddr(const sockaddr &addr) override
    {
        if (addr.sa_family != addr_.sin_family)
        {
            LOG_ERROR << "Different sa_family, can not set";
            return;
        }

        if (addr.sa_family == AF_INET)
        {
            auto &addr_ref = reinterpret_cast<const sockaddr_in &>(addr);
            addr_ = addr_ref;
        }
        else
        {
            auto &addr_ref = reinterpret_cast<const sockaddr_in6 &>(addr);
            addr6_ = addr_ref;
        }
    }

public:
    std::string GetIpStr() const;

    std::string GetIpPortStr() const;

    /// @brief It is users' responsibility to ensure the ip is indeed ipv4.
    uint32_t GetIpv4NetEndian() const;

    uint16_t GetPort() const { return endian::NetToHost(addr_.sin_port); }

    uint16_t GetPortNetEndian() const { return addr_.sin_port; }

    void SetSockAddr(const struct sockaddr_in &addr) { addr_ = addr; }

    void SetSockAddr6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }


    bool IsIpv4() const { return GetFamily() == AF_INET; }

    /// @brief Set Ipv6 ScopedID
    void SetScopeId(uint32_t scoped_id);   

    /// @brief Resolve the hostname to InetAddress.
    /// @attention It will execute syscall and is thread safe.
    /// @return true if resolve successfully otherwise false.
    static bool ResolveHost(StringProxy hostname, InetAddress *addr);

private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};
}   // namespace muzi

#endif  // MUZI_NET_INET_ADDRESS_H_
