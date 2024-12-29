#include "tcp_server.h"

#include "logger.h"
#include "event_loop.h"
#include "inet_address.h"

#include <stdio.h>
#include <string>
#include <unistd.h>

using namespace muzi;

void onConnection(const TcpConnectionPtr& conn)
{
  if (conn->IsConnected())
  {
    conn->SetTcpNoDelay(true);
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
  conn->Send(*buf);
  buf->RetriveAll();
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: server <address> <port> <threads>\n");
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::tid();
    gDefaultLogger.SetLogLevel(LogLevel::kWarn);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int threadCount = atoi(argv[3]);

    EventLoop loop;

    TcpServer server(&loop, listenAddr, "PingPong");

    server.SetConnectionCallback(onConnection);
    server.SetMessageCallback(onMessage);

    if (threadCount > 1)
    {
      server.SetThreadNum(threadCount);
    }

    server.Start();

    loop.Loop();
  }
}

