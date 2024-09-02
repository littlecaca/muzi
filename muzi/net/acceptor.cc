#include "acceptor.h"

#include <errno.h>
#include <functional>

#include <fcntl.h>

#include "current_thread.h"
#include "logger.h"
#include "socket.h"

namespace muzi
{

namespace
{
int OpenIdleFile()
{
    int fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        LOG_SYSFAT << "::open() fails";
    }
    return fd;
}

}   // internal linkage

Acceptor::Acceptor(EventLoop *loop, const AddressPtr &listen_addr, bool reuse_port)
    : loop_(loop),
      accept_socket_(socket::CreateNonBlockingSockOrDie()),
      local_addr_(listen_addr),
      chanel_(loop, accept_socket_.GetFd()),
      listening_(false),
      dummy_fd_(OpenIdleFile())
{
    accept_socket_.SetReuseAddr(true);
    if (reuse_port)
        accept_socket_.SetReusePort(true);
    accept_socket_.BindAddress(*listen_addr);
    chanel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor()
{
    // Acceptor will work in main loop, as with
    // the TcpServer. So this will be fine to
    // call these "in loop" function.
    chanel_.DisableAll();
    chanel_.Remove();
    socket::Close(dummy_fd_);
}

void Acceptor::Listen()
{
    loop_->AssertInLoopThread();
    LOG_TRACE << "Start listening in thread " << current_thread::tid();
    listening_ = true;
    accept_socket_.Listen();
    chanel_.EnableReading();
}

void Acceptor::HandleRead()
{
    loop_->AssertInLoopThread();

    int peer_sock;
    AddressPtr peer_addr(local_addr_->Copy());

    while ((peer_sock = socket::Accept(accept_socket_.GetFd(), peer_addr->GetAddr())))
    {
        // The special problem of accept()ing when you can't
        // Refer to this:
        // http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_accept_ing_wh
        // 
        // When the system's file descriptors are used up, the accept() will fail, but
        // it still keep the listening socket fd readable, which will cause our eventloop to
        // busy wait.
        // 
        // The graceful solution is that we can keep a dummy_fd_ to take up one file descriptor,
        // when this case occurs, we close it to release one and accept the new connection,
        // then we close the new conection, and set the dummy_fd_ again. This way we can 
        // gracefully close the new connection when the system's file descriptors are exhausted.
        //
        // This solution is not thread-safe, which means only one thread can run to accept new
        // connections using this solution. (Maybe a mutex_lock can make it thread-safe?)
        if (peer_sock < 0)
        {
            if (errno == EMFILE)
            {
                socket::Close(dummy_fd_);
                dummy_fd_ = ::accept(accept_socket_.GetFd(), nullptr, nullptr);
                socket::Close(dummy_fd_);
                dummy_fd_ = OpenIdleFile();
                continue;
            }
            else if (errno == EAGAIN)
            {
                break;
            }
        }   

        if (cb_)
        {
            cb_(peer_sock, peer_addr);
        }
        else
        {
            socket::Close(peer_sock);
        }
    }
}

} // namespace muzi
