#include "connector.h"

#include <errno.h>

#include "logger.h"

namespace muzi
{
Connector::Connector(EventLoop *loop, const AddressPtr &server_addr)
    : loop_(loop),
      server_addr_(server_addr),
      connect_(false),
      state_(kDisConnected),
      retry_delay_(kInitRetryDelayMs),
      timer_id_(-1),
      condition_(lock_)
{
}

Connector::~Connector()
{
    Stop();
    assert(!channel_);
}

void Connector::Start()
{
    connect_ = true;
    loop_->RunInLoop(std::bind(&Connector::StartInLoop, shared_from_this()));
}

void Connector::Stop()
{
    connect_ = false;
    loop_->RunInLoop(std::bind(&Connector::StopInLoop, shared_from_this()));
}

void Connector::StopAndWait()
{
    connect_ = false;
    if (loop_->IsInLoopThread())
    {
        StopInLoop();
    }
    else
    {
        MutexLockGuard guard(lock_);
        loop_->QueueInLoop(std::bind(&Connector::StopInLoop, this));
        condition_.Wait();
    }
}

void Connector::ReStartInLoop()
{
    loop_->AssertInLoopThread();
    connect_ = true;
    SetState(kDisConnected);
    retry_delay_ = kInitRetryDelayMs;
    StartInLoop();
}

void Connector::StartInLoop()
{
    loop_->AssertInLoopThread();
    if (connect_ && state_ == kDisConnected)
    {
        LOG_TRACE << "Start connecting to " << server_addr_->GetAddrStr();
        Connect();
    }
}

void Connector::StopInLoop()
{
    loop_->AssertInLoopThread();
    if (timer_id_ != -1)
    {
        loop_->CancelTimer(timer_id_);
    }
    
    if (!connect_ && state_ == kConnecting)
    {
        int sock_fd = RemoveAndResetChannel();
        // To close fd and SetState to kDisConnected.
        Retry(sock_fd);
    }
    condition_.Notify();
}

void Connector::HandleWrite()
{
    LOG_TRACE << "Connection establishing.";
    if (state_ == kConnecting)
    {
        int sock_fd = RemoveAndResetChannel();
        int err = socket::GetSocketError(sock_fd);
        if (err)
        {
            LOG_ERROR << "Conenctor::HandleWrite() " << err << GetError(err);
            Retry(sock_fd);
        }
        else if (socket::IsSelfConnect(sock_fd))
        {
            LOG_ERROR << "Connector::HandleWrite() Self connect";
            Retry(sock_fd);
        }
        else
        {
            SetState(kConnected);
            if (new_connection_callback_)
            {
                new_connection_callback_(sock_fd);
            }
            else
            {
                socket::Close(sock_fd);
            }
        }
    }
    else
    {
        assert(state_ == kDisConnected);
    }
}

void Connector::HandleError()
{
    LOG_TRACE << "Handling error";
    if (state_ == kConnecting)
    {  
        int sock_fd = RemoveAndResetChannel();
        int err = socket::GetSocketError(sock_fd);
        LOG_ERROR << "Error when establishing connection " << err << GetError(err);
        Retry(sock_fd);
    }
    else
    {
        assert(state_ == kDisConnected);
    }
}

void Connector::Connect()
{
    assert(state_ == kDisConnected);

    int sock_fd = socket::CreateNonBlockingSockOrDie();
    int res = socket::Connect(sock_fd, server_addr_->GetAddr());
    int saved_errno = res == 0 ? 0 : errno;
    switch (saved_errno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        Connecting(sock_fd);
        break;
    
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        Retry(sock_fd);
        break;
    
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERR << "Connector::Connect() fails to connect to " << server_addr_->GetAddrStr();
        socket::Close(sock_fd);
        break;

    default:
        LOG_SYSERR << "Connector::Connect() unknown error";
        socket::Close(sock_fd);
        break;
    }
}

void Connector::Connecting(int sock_fd)
{
    assert(!channel_);

    SetState(kConnecting);
    channel_.reset(new Channel(loop_, sock_fd));
    channel_->SetWriteCallback(std::bind(&Connector::HandleWrite, shared_from_this()));
    channel_->EnableWritting();
}

void Connector::Retry(int sock_fd)
{
    socket::Close(sock_fd);
    SetState(kDisConnected);
    if (connect_)
    {
        timer_id_ = loop_->RunAfter(retry_delay_ / 1000.0, std::bind(&Connector::StartInLoop, shared_from_this()));
        retry_delay_ = std::min(retry_delay_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_TRACE << "Stop retrying to connect";
    }
}

int Connector::RemoveAndResetChannel()
{
    int sock_fd = channel_->Getfd();
    channel_->DisableAll();
    channel_->Remove();
    // Can not reset here, we are now in channel->HanleEvent().
    loop_->QueueInLoop(std::bind(&Connector::ResetChannel, shared_from_this()));
    return sock_fd;
}

void Connector::ResetChannel()
{
    channel_.reset();
}

}   // namespace muzi
