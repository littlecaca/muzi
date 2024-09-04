#ifndef MUZI_NET_ADDRESS_H_
#define MUZI_NET_ADDRESS_H_

#include <string>
#include <memory>

#include <sys/socket.h>

namespace muzi
{
class Address;
typedef std::shared_ptr<Address> AddressPtr;

class Address
{
public:
    virtual const struct sockaddr *GetAddr() const = 0;
    virtual struct sockaddr *GetAddr() = 0;
    virtual void SetAddr(const sockaddr &addr) = 0;
    virtual std::string GetAddrStr() const  = 0;
    virtual sa_family_t GetFamily() const = 0;
    virtual AddressPtr Copy() const = 0;
    virtual int GetProtoFamily() const = 0;
    virtual size_t GetAddrSize() const = 0;
};


}   // namespace muzi

#endif  // MUZI_NET_ADDRESS_H_
