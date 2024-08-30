#ifndef MUZI_NET_TCP_CONNECTION_H_
#define MUZI_NET_TCP_CONNECTION_H_

#include <any>
#include <atomic>
#include <memory>
#include <netinet/tcp.h>
#include <string>

#include "buffer.h"
#include "condition.h"
#include "countdown_latch.h"
#include "channel.h"
#include "event_loop.h"
#include "inet_address.h"
#include "noncopyable.h"
#include "socket.h"
#include "timestamp.h"


namespace muzi
{
class TcpConnection;
typedef std::shared_ptr<TcpConnection>  TcpConnectionPtr;
// Callbacks related to TcpConnection
typedef std::function<void(const TcpConnectionPtr &)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr &, Buffer *buf, Timestamp time)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr &)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr &)> ErrorCallback;
typedef std::function<void(const TcpConnectionPtr &)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr &, size_t)> HighWaterCallback;


class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    static constexpr size_t kHighWaterMark = 64 * 1024 * 1024;

    TcpConnection(std::string name,
                  EventLoop *loop,
                  int sock_fd,
                  const InetAddress &peer_addr,
                  const InetAddress &local_addr);

    ~TcpConnection();

    /// @attention Not thread safe.
    void SetConnectionCallback(ConnectionCallback cb) { connection_callback_ = std::move(cb); }
    /// @attention Not thread safe.
    void SetMessageCallback(MessageCallback cb) { message_callback_ = std::move(cb); }
    /// @attention Not thread safe.
    void SetCloseCallback(CloseCallback cb) { close_callback_ = std::move(cb); }
    /// @brief Thread safe version of SetCloseCallback()
    void SetCloseCallbackAndWait(CloseCallback cb);

    /// @attention Not thread safe.
    void SetHighWaterCallback(HighWaterCallback cb) { high_water_callback_ = std::move(cb); }
    /// @attention Not thread safe.
    void SetWriteCompleteCallback(WriteCompleteCallback cb) { write_complete_callback_ = std::move(cb); }

    void ForceClose();

    void ForceCloseWithDelay(double seconds);
    
    /// @brief Half-close the output stream.
    void ShutDown();

    /// @brief Should only be called once.
    /// After set all the callbacks, the connection is ready to deal with events.
    /// @attention Because this operation must in loop, so we can not put it in
    /// ctor.
    void EstablishConnection();
    
    /// @brief Called when the connection be removed from the TcpServer's map,
    /// should only be called once.
    /// Because it is must in loop, we can not put it in the dtor.
    void DestroyConnection();

    /// @attention Not thread safe. 
    void SetTcpNoDelay(bool on)
    {
        socket_->SetTcpNoDelay(on);
    }

    /// @attention Not thread safe. 
    void SetContext(const std::any &context) { context_ = context; }
    /// @attention Not thread safe. 
    const std::any &GetContext() const { return context_; }
    /// @attention Not thread safe. 
    std::any &GetContext() { return context_; }

    Buffer &GetInputBuffer() { return input_buffer_; }
    Buffer &GetOutputBuffer() { return ouput_buffer_; }

    void StartRead();
    void StopRead();

    const std::string GetName() const { return name_; }

    EventLoop *GetLoop() const { return loop_; }

    const InetAddress &GetLocalAddress() const { return local_addr_; }
    const InetAddress &GetPeerAddress() const { return peer_addr_; }

    bool IsConnected() const { return state_ == kConnected; }
    bool IsDisConnected() const { return state_ == kDisConnected; }
    bool GetTcpInfo(struct tcp_info *) const;
    std::string GetTcpInfoString() const;

    void Send(const void *message, size_t len);
    void Send(StringProxy str);
    /// @attention This will retrive all data in buf.
    void Send(Buffer &buf);


private:
    enum ConnectionState
    {
        kConnecting,
        kConnected,
        kDisConnected,
        kDisConnecting,
    };

    void SetState(ConnectionState state)
    {
        state_ = state;
    }

    void HandleRead(Timestamp received_time);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void ForceCloseInLoop();
    void ShutDownInLoop();
    void SendInLoop(const StringProxy &str);
    void SendInLoop(const Buffer &buf);
    void SendInLoop(const void *first, size_t len);
    void StartReadInLoop();
    void StopReadInLoop();
    void SetCloseCallbackInLoop(CloseCallback cb, CountdownLatch *latch);

private:
    std::string name_;
    EventLoop *loop_;
    std::atomic<ConnectionState> state_;
    
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress peer_addr_;
    InetAddress local_addr_;

    size_t high_water_mark_;
    bool is_reading;

    Buffer input_buffer_;
    Buffer ouput_buffer_;

    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    WriteCompleteCallback write_complete_callback_;
    HighWaterCallback high_water_callback_;

    std::any context_;
};

} // namespace muzi

#endif  // MUZI_NET_TCP_CONNECTION_H_
