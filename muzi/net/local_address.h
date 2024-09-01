#ifndef MUZI_NET_LOCAL_ADDRESS_H_
#define MUZI_NET_LOCAL_ADDRESS_H_

#include <sys/un.h>

#include "socket_ops.h"
#include "address.h"

namespace muzi
{
class LocalAddress : Address
{
public:
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

    sa_family_t GetFamily() const override
    {
        return addr_.sun_family;
    }

private:
    sockaddr_un addr_;
};
}   // namespace muzi

#endif  // MUZI_NET_LOCAL_ADDRESS_H_
