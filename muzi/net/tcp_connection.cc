#include "tcp_connection.h"

namespace muzi
{
TcpConnection::TcpConnection(std::string name, 
                            EventLoop *loop, 
                            int sock_fd, 
                            const InetAddress &peer_addr)
    : name_(name),
      loop_(loop),
      state_(kConnecting),
      socket_(std::make_unique<Socket>(sock_fd)),   // std::unique_ptr<> from C++14
      channel_(std::make_unique<Channel>(loop, sock_fd)),
      peer_addr_(peer_addr)
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

void TcpConnection::HandleRead(Timestamp received_time)
{
}

void TcpConnection::HandleWrite()
{
}

void TcpConnection::HandleClose()
{
    LOG_TRACE << "TcpConnection::HandleClose(): " << name_;
    assert(state_ == kConnected);
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
