#ifndef MUZI_TCP_CLIENT_H_
#define MUZI_TCP_CLIENT_H_

#include <atomic>
#include <memory>
#include <string>

#include "connector.h"
#include "mutex.h"
#include "noncopyable.h"
#include "tcp_connection.h"

namespace muzi
{
class TcpClient : noncopyable
{
public:
    TcpClient(EventLoop *loop, 
              const InetAddress &server_addr,
              const std::string &name);
    ~TcpClient();

    void SetConnectionCallback(ConnectionCallback cb)
    {
        connection_callback_ = std::move(cb);
    }

    void SetMessageCallback(MessageCallback cb)
    {
        message_callback_ = std::move(cb);
    }
    
    void SetWriteCompleteCallback(WriteCompleteCallback cb)
    {
        write_complete_callback_ = std::move(cb);
    }
    
    void Connect();
    void DisConnect();
    void Stop();

    /// @brief Just make to_connect_ be false in loop_, and wait if necessaey.
    void DisableConnectAndWait();

    const TcpConnectionPtr &GetConnection() const;

    EventLoop *GetLoop() const { return loop_; }

    bool IsRetry() const { return is_retry; }
    
    /// @attention May have race condition.
    void EnableRetry() { is_retry = true; }

private:
    void NewConnection(int sock_fd);
    void RemoveConnection(const TcpConnectionPtr &conn);

    void DisableConnectInLoop();

private:
    EventLoop *loop_;
    std::shared_ptr<Connector> connector_;
    InetAddress server_addr_;
    std::string name_;
    std::atomic<int64_t> sequence_;
    bool to_connect_;           // atomic for internal
    std::atomic_bool is_retry;  // atomic
    mutable MutexLock lock_;
    Condition condition_;
    TcpConnectionPtr connection_ GUARDED_BY(lock_);

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
};

}   // namespace muzi

#endif  // MUZI_TCP_CLIENT_H_
