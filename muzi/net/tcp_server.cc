#include "tcp_server.h"

#include <functional>

#include "socket.h"

namespace muzi
{

namespace
{
void DefaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_TRACE << conn->GetLocalAddress().GetIpPortStr() << " >> "
              << conn->GetPeerAddress().GetIpPortStr() << " is "
              << (conn->IsConnected() ? "UP" : "DOWN");
}

void DefaultMessageCallback(const TcpConnectionPtr &conn,
                            Buffer *buf,
                            Timestamp time)
{
    // Do nothing but clear the input buffer.
    buf->RetriveAll();
}

}   // internal linkage

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listen_addr, 
    const std::string &name, bool reuse_port)
    : name_(name),
      loop_(loop),
      acceptor_(std::make_unique<Acceptor>(loop, listen_addr, reuse_port)),
      started_(false),
      conn_sequence_(0),
      thread_pool_(std::make_unique<EventLoopThreadPool>(loop, name + "EventThread")),
      connection_callback_(&DefaultConnectionCallback),
      message_callback_(&DefaultMessageCallback)
{
    acceptor_->SetNewConnectionCallBack(std::bind(&TcpServer::NewConnection, 
        this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpServer " << name_ << " is being destructing";
    for (auto &[name, conn_ptr] : connections_) // C++17
    {
        conn_ptr->GetLoop()->RunInLoop(
            std::bind(&TcpConnection::DestroyConnection, conn_ptr));
    }
}

void TcpServer::Start()
{
    if (started_.exchange(true) == false)
    {
        thread_pool_->Start(thread_init_callback_);

        assert(acceptor_->IsListening());
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
    }
}

void TcpServer::SetThreadNum(size_t num) const
{
    thread_pool_->SetThreadNum(num);
}

void TcpServer::NewConnection(int sock_fd, const InetAddress &peer_addr)
{
    loop_->AssertInLoopThread();

    std::string conn_name = name_ + "#" + std::to_string(conn_sequence_++);

    LOG_INFO << "TcpServer " << name_ << " accept new connection ["
             << conn_name << "] from " << peer_addr.GetIpPortStr();

    // Here we can give it other loops to hand it over.
    EventLoop *io_loop = thread_pool_->GetNextLoop();

    TcpConnectionPtr conn(std::make_shared<TcpConnection>
        (conn_name, io_loop, sock_fd, peer_addr, acceptor_->GetLocalAddr()));
    
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

    io_loop->RunInLoop(std::bind(&TcpConnection::EstablishConnection, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{
    // Due to that RemoveConnection will change TcpServer::connections_,
    // while this method may be called by other loops' HandleEvent(),
    // so to ensure thread-safe, we need to execute this opertion in
    // the TcpServer's loop.
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->AssertInLoopThread();
    
    LOG_TRACE << "Removing connection "
              << conn->GetName();

    size_t n = connections_.erase(conn->GetName());
    assert(n == 1); (void)n;
    // Here if we do not put the DestroyConnection() to io_loop pending functors list
    // after execution, the connection may be destroyed, and now is actually
    // in channel_->HandleEvent(), so the connection must not be destroyed here.
    //
    // But with channel->Tie(), we have guaranteed that the conn will not be destroyed
    // afther this. So we can use RunInLoop rather than having to use QueueInLoop.
    EventLoop *io_loop = conn->GetLoop();
    io_loop->RunInLoop(std::bind(&TcpConnection::DestroyConnection, conn));
}

}   // namespace muzi
