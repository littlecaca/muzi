#include "poller.h"

#include <stdlib.h>

#include "epoll_poller.h"

namespace muzi
{
Poller *Poller::NewDefaultPoller(EventLoop *loop)
{
    if (::getenv("MUZI_USE_POLL"))
    {
        // FIXME
        assert(false);
    }
    else
    {
        return new EpollPoller(loop);
    }
}

}   // namespace muzi
