#ifndef MUZI_NET_SOCKET_OPS_H_
#define MUZI_NET_SOCKET_OPS_H_

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>

// REMOVE ME
#include "logger.h"

namespace muzi
{
namespace socket
{
// Very low level operations used by the implementations of other modules

inline const struct sockaddr *SockAddrCast(const struct sockaddr_in6 *addr6)
{
    return reinterpret_cast<const struct sockaddr *>(addr6);
}

inline struct sockaddr *SockAddrCast(struct sockaddr_in6 *addr6)
{
    return reinterpret_cast<struct sockaddr *>(addr6);
}

inline const struct sockaddr_in *SockAddrInCast(const struct sockaddr *addr)
{
    return reinterpret_cast<const struct sockaddr_in *>(addr);
}

inline const struct sockaddr_in6 *SockAddrIn6Cast(const struct sockaddr *addr)
{
    return reinterpret_cast<const struct sockaddr_in6 *>(addr);
}

void AbortNotInterNet(const struct sockaddr *addr);

/// @brief Convert and write ip, port to binary netwrok form.
void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in *addr);

/// @brief Convert and write ip, port to binary netwrok form.
void FromIpPort(const char *ip, uint16_t port, struct sockaddr_in6 *addr6);

/// @brief Write ip:port presentation to buf.
void ToIpPort(char *buf, int buf_len, const struct sockaddr *addr);

/// @brief Write ip presentation to buf.
void ToIp(char *buf, int buf_len, const struct sockaddr *addr);

/// @brief Write port presentation to buf.
void ToPort(char *buf, int buf_len, const struct sockaddr *addr);

void Close(int fd);

bool GetTcpInfo(int sock_fd, tcp_info *info);

bool GetTcpInfoString(int sock_fd, char *buf, int len);

bool BindAddress(int sock_fd, const struct sockaddr *addr);

bool Listen(int sock_fd);

int Connect(int sock_fd, const struct sockaddr *addr);

int Accept(int sock_fd, struct sockaddr *addr);

bool ShutDownOnWrite(int sock_fd);

bool SetSockOpt(int sock_fd, int level, int opt, bool on);

int CreateBlockingSockOrDie();

int CreateNonBlockingSockOrDie();

struct sockaddr_in6 GetLocalAddr(int sock_fd);

struct sockaddr_in6 GetPeerAddr(int sock_fd);

int GetSocketError(int sock_fd);

inline ssize_t Write(int sock_fd, const void *buf, size_t len)
{
    // REMOVE ME
    LOG_DEBUG_U(gStdioLogger) << (const char *)buf;
    return ::write(sock_fd, buf, len);
}

bool IsSelfConnect(int sock_fd);

}   // namespace socket
}   // namespace muzi


#endif  // MUZI_NET_SOCKET_OPS_H_
