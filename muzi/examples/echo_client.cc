#include "tcp_client.h"

#include <iostream>
#include <string>

#include "logger.h"
#include "async_outputer.h"
#include "event_loop_thread_pool.h"

using namespace muzi;

void OnConnection(const TcpConnectionPtr &conn)
{
    if (conn->IsConnected())
    {
        LOG_INFO << "Connect successfully";
    }
    else
    {
        LOG_INFO << "Connection close";
    }
}

void OnMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time)
{
    std::cout << "Message from server:\n" + buffer->RetriveAllAsString() + "\n";
}


int main(int argc, char const *argv[])
{
    AsyncOutputer outputer("echo_client");
    gDefaultLogger.SetOutputer(&outputer);
    outputer.Start();

    EventLoopThread loop_thread;
    
    EventLoop *loop = loop_thread.StartLoop();
    TcpClient client(loop, InetAddress("127.0.0.1", 2555), "EchoClient");

    client.SetConnectionCallback(OnConnection);
    client.SetMessageCallback(OnMessage);
    client.Connect();

    TcpConnectionPtr conn = client.GetConnection();
    
    std::string line;
    std::cout << "Msg: " << std::flush;
    while ((std::getline(std::cin, line)) && conn->IsConnected())
    {
        LOG_INFO << "Get input: " << line;
        if (line == "Q" || line == "q")
            break;
        conn->Send(line);
        std::cout << "Msg: " << std::flush;
    }
    return 0;
}
