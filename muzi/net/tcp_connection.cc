#include "tcp_connection.h"

#include "weak_callback.h"

namespace muzi
{


TcpConnection::TcpConnection(std::string name, 
                            EventLoop *loop, 
                            int sock_fd, 
                            const InetAddress &peer_addr,
                            const InetAddress &local_addr)
    : name_(name),
      loop_(loop),
      state_(kConnecting),
      socket_(std::make_unique<Socket>(sock_fd)),   // std::unique_ptr<> from C++14
      channel_(std::make_unique<Channel>(loop, sock_fd)),
      peer_addr_(peer_addr),
      local_addr_(local_addr),
      high_water_mark_(kHighWaterMark),
      is_reading(true)
{
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));
    channel_->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
    channel_->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
    channel_->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
    LOG_TRACE << "Connectoin established: " << name_ << " with socket fd " << socket_->GetFd();

    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_TRACE << "Connection destroyed: " << name_;
    assert(state_ == kDisConnected);
}

void TcpConnection::ForceCloseInLoop()
{
    loop_->AssertInLoopThread();
    if (state_ == kConnected || state_ == kDisConnecting)
    {
        HandleClose();
    }
}

void TcpConnection::ShutDownInLoop()
{
    loop_->AssertInLoopThread();
    if (!channel_->IsWritting())
    {
        socket_->ShutDownOnWrite();
    }
}

void TcpConnection::SendInLoop(const StringProxy &str)
{
    SendInLoop(str.begin(), str.size());
}

void TcpConnection::SendInLoop(const Buffer &buf)
{
    loop_->AssertInLoopThread();
    if (state_ == kDisConnected)
    {
        LOG_WARN << "Disconnected, give up writting";
        return;
    }

    // If the data in output buffer will reach high water mark,
    // try calling the high water callback.
    size_t old_len = ouput_buffer_.ReadableBytes();
    size_t new_len = buf.ReadableBytes();
    if (old_len + new_len >= high_water_mark_
        && old_len < high_water_mark_
        && high_water_callback_)
    {
        /// @attention Do need to put this to loop's functors quque?
        high_water_callback_(shared_from_this(), old_len + new_len);
    }

    ouput_buffer_.Append(buf);
    if (!channel_->IsWritting())
    {
        channel_->EnableWritting();
    }
}

void TcpConnection::SendInLoop(const void *first, size_t len)
{
    loop_->AssertInLoopThread();

    ssize_t written = 0;
    size_t remaining = len;

    if (state_ == kDisConnected)
    {
        LOG_WARN << "Disconnected, give up writting";
        return;
    }
    // If no thing in output queue, try writting directly.
    if (!channel_->IsWritting() && ouput_buffer_.ReadableBytes() == 0)
    {
        written = socket::Write(channel_->Getfd(), first, len);
        if (written > 0)
        {
            remaining -= written;
            if (len == 0 && write_complete_callback_)
            {
                /// @attention Do need to put this to loop's functors quque?
                write_complete_callback_(shared_from_this());
            }
        }
        else
        {
            written = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcoConnection::SendInLoop()";
            }
        }
    }
    
    // Write the remaining bytes to output buffer.
    if (remaining > 0)
    {
        // If the bytes in buffer will reach high water mark,
        // try calling high water callback.
        size_t old_len = ouput_buffer_.ReadableBytes();
        if (remaining + old_len >= high_water_mark_
            && old_len < high_water_mark_
            && high_water_callback_)
        {
            /// @attention Do need to put this to loop's functors quque?
            high_water_callback_(shared_from_this(), old_len + remaining);
        }

        ouput_buffer_.Append(static_cast<const char *>(first) + written, len);
        if (!channel_->IsWritting())
        {
            channel_->EnableWritting();
        }
    }
}

void TcpConnection::ForceClose()
{
    if (state_ == kConnected || state_ == kDisConnecting)
    {
        loop_->RunInLoop(std::bind(&TcpConnection::ForceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::ForceCloseWithDelay(double seconds)
{
    if (state_ == kConnected || state_ == kDisConnecting)
    {
        // SetState(kDisConnecting);
        loop_->RunAfter(
            seconds,
            MakeWeakCallback(shared_from_this(), 
                &TcpConnection::ForceCloseInLoop)
        );
    }
}

void TcpConnection::ShutDown()
{
    ConnectionState expected = kConnected;
    if (state_.compare_exchange_strong(expected, kDisConnecting))
    {
        loop_->RunInLoop(std::bind(&TcpConnection::ShutDownInLoop, shared_from_this()));
    }
}

void TcpConnection::EstablishConnection()
{
    loop_->AssertInLoopThread();
    
    assert(close_callback_);
    assert(connection_callback_);
    assert(state_ == kConnecting);
    SetState(kConnected);
    channel_->Tie(shared_from_this());
    channel_->EnableReading();
    connection_callback_(shared_from_this());
}

void TcpConnection::DestroyConnection()
{
    loop_->AssertInLoopThread();

    assert(state_ == kDisConnected);
    channel_->Remove();
}

void TcpConnection::StartRead()
{
    loop_->RunInLoop(std::bind(&TcpConnection::StartReadInLoop, shared_from_this()));
}

void TcpConnection::StartReadInLoop()
{
    loop_->AssertInLoopThread();

    if ((!is_reading || !channel_->IsReading())
        && state_ != kConnecting && state_ != kDisConnected)
    {
        channel_->EnableReading();
        is_reading = true;
    }
}

void TcpConnection::StopRead()
{
    loop_->RunInLoop(std::bind(&TcpConnection::StopReadInLoop, shared_from_this()));
}

void TcpConnection::StopReadInLoop()
{
    loop_->AssertInLoopThread();
    if ((is_reading || channel_->IsReading())
        && state_ != kDisConnected)
    {
        channel_->DisableReading();
        is_reading = false;
    }
}

bool TcpConnection::GetTcpInfo(tcp_info *info) const
{
    return socket::GetTcpInfo(channel_->Getfd(), info);
}

std::string TcpConnection::GetTcpInfoString() const
{
    return socket_->GetTcpInfo();
}

void TcpConnection::Send(const void *message, size_t len)
{
    Send(StringProxy(static_cast<const char *>(message), len));
}

void TcpConnection::Send(StringProxy str)
{
    if (state_ == kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(str.begin(), str.size());
        }
        else
        {
            // Asyscronization call, so we must copy the message.
            void (TcpConnection::*fp)(const StringProxy &) 
                = &TcpConnection::SendInLoop;
            loop_->RunInLoop(std::bind(fp, shared_from_this(), str.ToString()));
        }
    }
}

void TcpConnection::Send(Buffer &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(buf);
        }
        else
        {
            // Ayscronization call, so we must copy the message.
            void (TcpConnection::*fp)(const StringProxy &)
                = &TcpConnection::SendInLoop;
            loop_->RunInLoop(std::bind(fp, shared_from_this(), buf.PeekAllAsString()));
        }
    }
}

void TcpConnection::HandleRead(Timestamp received_time)
{
    loop_->AssertInLoopThread();

    int saved_errno = 0;
    ssize_t n = input_buffer_.ReadFd(channel_->Getfd(), &saved_errno);
    if (n < 0)
    {
        errno = saved_errno;
        LOG_SYSERR << "TcpConnection::HandleRead()";
    }
    else if (n == 0)
    {
        HandleClose();
    }
    else
    {
        message_callback_(shared_from_this(), &input_buffer_, received_time);
    }
}

void TcpConnection::HandleWrite()
{
    loop_->AssertInLoopThread();

    if (channel_->IsWritting())
    {
        int saved_errno = 0;
        ssize_t n = ouput_buffer_.WriteFd(channel_->Getfd(), &saved_errno);
        if (n < 0)
        {
            errno = saved_errno;
            LOG_SYSERR << "TcpConnection::HandleWrite()";
        }
        else
        {
            if (ouput_buffer_.ReadableBytes() == 0)
            {
                channel_->DisableWritting();
                if (write_complete_callback_)
                {
                    write_complete_callback_(shared_from_this());
                }
                if (state_ == kDisConnecting)
                {
                    ShutDownInLoop();
                }
            }
        }
    }
    else
    {
        LOG_TRACE << "Connection fd = " << channel_->Getfd()
                  << " is down, no more writting";
    }
}

void TcpConnection::HandleClose()
{
    LOG_TRACE << "TcpConnection::HandleClose(): " << name_;
    assert(state_ == kConnected || state_ == kDisConnecting);
    loop_->AssertInLoopThread();

    SetState(kDisConnected);
    channel_->DisableAll();

    TcpConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
}

void TcpConnection::HandleError()
{
    int savederr = errno;
    errno = socket::GetSocketError(socket_->GetFd());
    LOG_ERROR << "TcpConnection::HandleError(): " << name_;
    errno = savederr;
}

}   // namespace muzi
