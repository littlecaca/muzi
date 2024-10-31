#include "event_loop.h"
#include "tcp_client.h"
#include "logger.h"
#include "current_thread.h"

using namespace muzi;

void SayHello(const TcpConnectionPtr &conn)
{
    LOG_INFO << "Hello";
    gDefaultOutputer.Flush();
}

int main(int argc, char const *argv[])
{
    LOG_INFO << "Test start in thread " << current_thread::tid();

    EventLoop loop;
    InetAddress server_addr("127.0.0.1", 2);

    TcpClient client(&loop, server_addr, "myclient");

    client.SetConnectionCallback(&SayHello);
    client.Connect();
    
    current_thread::SleepUsec(1000 * 1000);
    return 0;
}
