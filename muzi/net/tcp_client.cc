#include "tcp_client.h"

#include "logger.h"

namespace muzi
{
TcpClient::TcpClient(EventLoop *loop, 
                     const InetAddress &server_addr, 
                     const std::string &name)
    : loop_(loop),
      connector_(std::make_shared<Connector>(loop, server_addr)),
      server_addr_(server_addr),
      name_(name),
      to_connect_(false),
      is_retry(false)
{
    connector_->SetNewConnectionCallback(std::bind(&TcpClient::NewConnection,
         this, std::placeholders::_1));
}

TcpClient::~TcpClient()
{
    LOG_TRACE << name_ << " destroying connection and connector";

    // release connection_
    TcpConnectionPtr conn;
    
    {
        MutexLockGuard guard(lock_);
        conn = std::move(connection_);
    }

    if (conn)
    {
        loop_->RunInLoop(std::bind(&TcpConnection::DestroyConnection, conn));
    }

    // It will be safe if connector_ is in loop_.
    // Make sure the connector and Tcpclient are in the same thread.
    connector_->Stop();
}

void TcpClient::Connect()
{
    loop_->AssertInLoopThread();

    to_connect_ = true;
    connector_->Start();
}

void TcpClient::DisConnect()
{
    loop_->AssertInLoopThread();

    to_connect_ = false;
    {
        MutexLockGuard guard(lock_);
        if (connection_)
        {
            connection_->ShutDown();
        }
    }
}

void TcpClient::Stop()
{
    loop_->AssertInLoopThread();

    to_connect_ = false;
    connector_->Stop();
}

/// @brief Called by connector_.
void TcpClient::NewConnection(int sock_fd)
{
    loop_->AssertInLoopThread();

    std::string conn_name = name_ + "#" + std::to_string(sequence_++);

    TcpConnectionPtr conn = std::make_shared<TcpConnection>(conn_name,
        loop_, sock_fd, server_addr_, socket::GetLocalAddr(sock_fd));

    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpClient::RemoveConnection,
         this, std::placeholders::_1));

    LOG_TRACE << "New connection " << conn_name << " to " << server_addr_.GetIpPortStr()
              << " is establishing.";

    {
        MutexLockGuard guard(lock_);
        connection_ = std::move(conn);
    }

    connection_->EstablishConnection();
}

/// @brief Called by connection_.
void TcpClient::RemoveConnection(const TcpConnectionPtr &conn)
{
    loop_->AssertInLoopThread();

    LOG_TRACE << "Removing connection " << conn->GetName();

    {
        MutexLockGuard guard(lock_);
        if (conn == connection_)
        {
            connection_.reset();
        }
    }

    loop_->RunInLoop(std::bind(&TcpConnection::DestroyConnection, conn));

    if (to_connect_ && is_retry)
    {
        LOG_TRACE << "Reconnecting to " << server_addr_.GetIpPortStr();
        connector_->ReStartInLoop();
    }
}

}   // namespace muzi
