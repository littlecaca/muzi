#ifndef MUZI_NET_LOCAL_ADDRESS_H_
#define MUZI_NET_LOCAL_ADDRESS_H_

#include <assert.h>

#include <sys/un.h>

#include "socket_ops.h"
#include "address.h"

namespace muzi
{
class LocalAddress : public Address
{
public:
    LocalAddress(const char *path)
    {
        addr_.sun_family = AF_LOCAL;
        ::strncpy(addr_.sun_path, path, sizeof addr_.sun_path - 1);
        DEBUGINFO << addr_.sun_path;
    }

    /// @attention LocalAddress can not own the local sock file
    ~LocalAddress()
    {
    }

    const struct sockaddr *GetAddr() const override
    {
        return reinterpret_cast<const sockaddr *>(&addr_);
    }

    struct sockaddr *GetAddr() override
    {
        return reinterpret_cast<sockaddr *>(&addr_);
    }

    std::string GetAddrStr() const override
    {
        return addr_.sun_path;
    }

    size_t GetAddrSize() const override
    {
        return sizeof(sockaddr_un);
    }

    sa_family_t GetFamily() const override
    {
        return addr_.sun_family;
    }

    int GetProtoFamily() const override
    {
        return PF_LOCAL;
    }

    AddressPtr Copy() const override
    {
        return std::make_shared<LocalAddress>(*this);
    }

    void SetAddr(const sockaddr &addr) override
    {
        if (addr.sa_family != addr_.sun_family)
        {
            LOG_ERROR << "Different sa_family, can not set";
            return;
        }
        const char *path = reinterpret_cast<const sockaddr_un &>(addr).sun_path;
        ::memcpy(addr_.sun_path, path, sizeof addr_.sun_path);
    }

private:
    sockaddr_un addr_;
};
}   // namespace muzi

#endif  // MUZI_NET_LOCAL_ADDRESS_H_
