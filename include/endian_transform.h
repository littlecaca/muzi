#ifndef MUZI_NET_ENDIAN_H_
#define MUZI_NET_ENDIAN_H_

#include <endian.h>
#include <arpa/inet.h>
#include <stdint.h>

namespace muzi
{
namespace endian
{
template <typename T>
inline T HostToNet(T host)
{
    static_assert(false, "Only accept uint64,32,16,8_t");
}

template<>
inline uint8_t HostToNet(uint8_t net8)
{
    return net8;
}

template <>
inline uint16_t HostToNet(uint16_t host16)
{
    return htobe16(host16);
}

template <>
inline uint32_t HostToNet(uint32_t host32)
{
    return htobe32(host32);
}

template <>
inline uint64_t HostToNet(uint64_t host64)
{
    return htobe64(host64);
}

template <typename T>
inline T NetToHost(T net)
{
    static_assert(false, "Only accept uint64,32,16,8_t");
}

template<>
inline uint8_t NetToHost(uint8_t net8)
{
    return net8;
}

template <>
inline uint16_t NetToHost(uint16_t net16)
{
    return be16toh(net16);
}

template <>
inline uint32_t NetToHost(uint32_t net32)
{
    return be32toh(net32);
}

template <>
inline uint64_t NetToHost(uint64_t net64)
{
    return be64toh(net64);
}


}   // namespace endian

}   // namespace muzi

#endif  // MUZI_NET_ENDIAN_H_
