#include "tcp_client.h"

#include "logger.h"
#include "event_loop.h"
#include "event_loop_thread_pool.h"
#include "inet_address.h"

#include <atomic>

#include <stdio.h>
#include <unistd.h>
#include <string>

using namespace muzi;
using std::string;
using namespace std::placeholders;

class Client;

class Session : noncopyable
{
 public:
  Session(EventLoop* loop,
          const InetAddress& serverAddr,
          const string& name,
          Client* owner)
    : client_(loop, serverAddr, name),
      owner_(owner),
      bytesRead_(0),
      bytesWritten_(0),
      messagesRead_(0)
  {
    client_.SetConnectionCallback(
        std::bind(&Session::onConnection, this, _1));
    client_.SetMessageCallback(
        std::bind(&Session::onMessage, this, _1, _2, _3));
  }

  void start()
  {
    client_.Connect();
  }

  void stop()
  {
    client_.DisConnect();
  }

  int64_t bytesRead() const
  {
     return bytesRead_;
  }

  int64_t messagesRead() const
  {
     return messagesRead_;
  }

 private:

  void onConnection(const TcpConnectionPtr& conn);

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    ++messagesRead_;
    bytesRead_ += buf->ReadableBytes();
    bytesWritten_ += buf->ReadableBytes();
    conn->Send(*buf);
    buf->RetriveAll();
  }

  TcpClient client_;
  Client* owner_;
  int64_t bytesRead_;
  int64_t bytesWritten_;
  int64_t messagesRead_;
};

class Client : noncopyable
{
 public:
  Client(EventLoop* loop,
         const InetAddress& serverAddr,
         int blockSize,
         int sessionCount,
         int timeout,
         int threadCount)
    : loop_(loop),
      threadPool_(loop, "pingpong-client"),
      sessionCount_(sessionCount),
      timeout_(timeout),
      numConnected_(0)
  {
    loop->RunAfter(timeout, std::bind(&Client::handleTimeout, this));
    if (threadCount > 1)
    {
      threadPool_.SetThreadNum(threadCount);
    }
    threadPool_.Start();

    for (int i = 0; i < blockSize; ++i)
    {
      message_.push_back(static_cast<char>(i % 128));
    }

    for (int i = 0; i < sessionCount; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "C%05d", i);
      Session* session = new Session(threadPool_.GetNextLoop(), serverAddr, buf, this);
      session->start();
      sessions_.emplace_back(session);
    }
  }

  const string& message() const
  {
    return message_;
  }

  void onConnect()
  {
    if (numConnected_.fetch_add(1, std::memory_order_acq_rel) == sessionCount_ - 1)
    {
      LOG_WARN << "all connected";
    }
  }

  void onDisconnect(const TcpConnectionPtr& conn)
  {
    if (numConnected_.fetch_sub(1, std::memory_order_acq_rel) == 1)
    {
      LOG_WARN << "all disconnected";

      int64_t totalBytesRead = 0;
      int64_t totalMessagesRead = 0;
      for (const auto& session : sessions_)
      {
        totalBytesRead += session->bytesRead();
        totalMessagesRead += session->messagesRead();
      }
      LOG_WARN << totalBytesRead << " total bytes read";
      LOG_WARN << totalMessagesRead << " total messages read";
      LOG_WARN << static_cast<double>(totalBytesRead) / static_cast<double>(totalMessagesRead)
               << " average message size";
      LOG_WARN << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
               << " MiB/s throughput";
      conn->GetLoop()->QueueInLoop(std::bind(&Client::quit, this));
    }

  }

 private:

  void quit()
  {
    loop_->QueueInLoop(std::bind(&EventLoop::Quit, loop_));
  }

  void handleTimeout()
  {
    LOG_WARN << "stop";
    for (auto& session : sessions_)
    {
      session->stop();
    }
  }

  EventLoop* loop_;
  EventLoopThreadPool threadPool_;
  int sessionCount_;
  int timeout_;
  std::vector<std::unique_ptr<Session>> sessions_;
  string message_;
  std::atomic<int> numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->IsConnected())
  {
    // LOG_WARN << "new connection";
    conn->SetTcpNoDelay(true);
    conn->Send(owner_->message());
    owner_->onConnect();
  }
  else
  {
    // LOG_WARN << "connection closed";
    owner_->onDisconnect(conn);
  }
}

int main(int argc, char* argv[])
{
  if (argc != 7)
  {
    fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
    fprintf(stderr, "<sessions> <time>\n");
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::tid();
    gDefaultLogger.SetLogLevel(LogLevel::kWarn);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    int threadCount = atoi(argv[3]);
    int blockSize = atoi(argv[4]);
    int sessionCount = atoi(argv[5]);
    int timeout = atoi(argv[6]);

    EventLoop loop;
    InetAddress serverAddr(ip, port);

    Client client(&loop, serverAddr, blockSize, sessionCount, timeout, threadCount);
    loop.Loop();
  }
}

