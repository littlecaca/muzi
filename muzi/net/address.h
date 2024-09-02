#ifndef MUZI_NET_ADDRESS_H_
#define MUZI_NET_ADDRESS_H_

#include <string>

#include <sys/socket.h>

namespace muzi
{
class Address
{
public:
    virtual const struct sockaddr *GetAddr() const = 0;
    virtual struct sockaddr *GetAddr() = 0;
    virtual std::string GetAddrStr() const  = 0;
    virtual sa_family_t GetFamily() const = 0;

};

}   // namespace muzi

#endif  // MUZI_NET_ADDRESS_H_
