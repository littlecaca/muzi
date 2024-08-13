#include "tcp_server.h"

#include <functional>

#include "socket.h"

namespace muzi
{

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr, 
    const std::string &name, bool reuse_port)
    : name_(name),
      loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listen_addr, reuse_port)),
      started_(false),
      conn_sequence_(0)
{
    acceptor_->SetNewConnectionCallBack(std::bind(&TcpServer::NewConnection, 
        this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::Start()
{
    if (started_.exchange(true) == false)
    {
        assert(acceptor_->IsListening());
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
    }
}

void TcpServer::NewConnection(int sock_fd, const InetAddress &peer_addr)
{
    loop_->AssertInLoopThread();

    std::string conn_name = name_ + "#" + std::to_string(conn_sequence_++);

    LOG_INFO << "TcpServer " << name_ << " accept new connection ["
             << conn_name << "] from " << peer_addr.GetIpPortStr();

    TcpConnectionPtr conn(std::make_shared<TcpConnection>
        (conn_name, loop_, sock_fd, peer_addr));
    
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    conn->EstablishConnection();
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{
    loop_->AssertInLoopThread();
    
    size_t n = connections_.erase(conn->GetName());
    assert(n == 1); (void)n;
    loop_->RunInLoop(std::bind(&TcpConnection::DestroyConnection, conn));
}

}   // namespace muzi
