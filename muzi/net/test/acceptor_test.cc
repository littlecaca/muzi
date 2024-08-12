#include "acceptor.h"

#include <fcntl.h>
#include <unistd.h>

#include "event_loop_thread.h"
#include "logger.h"

void NewConnection(int sock_fd, const muzi::InetAddress &peer_addr)
{
    LOG_INFO << "Accept new connection from " << peer_addr.GetIpPortStr();
    char buf[] = "Hello, how are you doing?";
    ::write(sock_fd, buf, sizeof buf);
    muzi::socket::Close(sock_fd);
}


int main(int argc, char const *argv[])
{
    LOG_INFO << "main() pid = " << getpid();

    muzi::EventLoopThread loop_thread;
    muzi::EventLoop *loop =  loop_thread.StartLoop();


    muzi::InetAddress listen_addr(9981);
    muzi::Acceptor *acceptor = new muzi::Acceptor(loop, listen_addr);
    acceptor->SetNewConnectionCallBack(&NewConnection);

    loop->RunInLoop([&acceptor]() { acceptor->Listen(); });

    sleep(1);
    muzi::InetAddress client_addr("127.0.0.1", 9981);

    muzi::Socket peer_sock(muzi::socket::CreateBlockingSockOrDie());
    peer_sock.Connect(client_addr);


    sleep(1);

    loop->RunInLoop([&acceptor]() { delete acceptor; });


    return 0;
}
