#include "tcp_client.h"
#include "logger.h"

using namespace muzi;

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->IsConnected())
    {
        LOG_INFO << "Connect successfully";
        conn->Send("How are you?");
        conn->Send("Who are you?");
    }
    else
    {
        LOG_INFO << "Connection close";
    }
}

void OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    LOG_INFO << "Message from server:\n"
             << buffer->PeekAllAsString();
}


int main(int argc, char const *argv[])
{
    EventLoop loop;
    TcpClient client(&loop, InetAddress("127.0.0.1", 2555), "EchoClient");
    gDefaultOutputer.Flush();

    client.SetConnectionCallback(OnConnection);
    gDefaultOutputer.Flush();

    client.SetMessageCallback(OnMessage);
    gDefaultOutputer.Flush();

    client.Connect();
    gDefaultOutputer.Flush();

    loop.Loop();

    return 0;
}
