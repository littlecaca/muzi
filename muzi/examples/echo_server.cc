#include "tcp_server.h"

#include <functional>

#include "async_outputer.h"
#include "buffer.h"
#include "inet_address.h"
#include "logger.h"

using namespace muzi;
using namespace std::placeholders;

class EchoServer
{
public:
    EchoServer(EventLoop *loop, const InetAddress &listen_addr)
        : loop_(loop),
          server_(loop, listen_addr, "EchoServer")
    {
        server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
        server_.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
    }

    void Start();

private:
    void OnConnection(const TcpConnectionPtr &conn);
    
    void OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time);

private:
    EventLoop *loop_;
    TcpServer server_;
};

void EchoServer::Start()
{
    server_.Start();
}

void EchoServer::OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->IsConnected())
    {
        LOG_INFO << "Accept new connection " << conn->GetName()
                 << " from " << conn->GetPeerAddress().GetIpPortStr();
    }
    else
    {
        LOG_INFO << "Close connnection " << conn->GetName();
    }
}

void EchoServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    DEBUGINFO("EchoServer::OnMessage()");
    conn->Send(*buffer);
    buffer->RetriveAll();
}

int main(int argc, char const *argv[])
{
    // Set logger
    // AsyncOutputer outputer("echo_log");
    // gDefaultLogger.SetOutputer(&outputer);
    // outputer.Start();

    EventLoop loop;
    EchoServer server(&loop, InetAddress("127.0.0.1", 2555));
    server.Start();

    loop.Loop();

    return 0;
}
