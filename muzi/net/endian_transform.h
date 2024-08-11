#ifndef MUZI_NET_ENDIAN_H_
#define MUZI_NET_ENDIAN_H_

#include <endian.h>
#include <arpa/inet.h>
#include <stdint.h>

namespace muzi
{
namespace endian
{
inline uint16_t HostToNet16(uint16_t host16)
{
    return htobe16(host16);
}

inline uint32_t HostToNet32(uint32_t host32)
{
    return htobe32(host32);
}

inline uint64_t HostToNet64(uint64_t host64)
{
    return htobe64(host64);
}

inline uint16_t NetToHost16(uint16_t net16)
{
    return be16toh(net16);
}

inline uint32_t NetToHost32(uint32_t net32)
{
    return be32toh(net32);
}

inline uint64_t NetToHost64(uint64_t net64)
{
    return be64toh(net64);
}

}   // namespace endian

}   // namespace muzi

#endif  // MUZI_NET_ENDIAN_H_
