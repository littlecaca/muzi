#include "event_loop.h"

#include <sys/poll.h>

#include "logger.h"

namespace muzi
{
namespace
{
thread_local EventLoop *t_loop_in_this_thread = nullptr;
}   // internal linkage


EventLoop::EventLoop() 
    : tid_(current_thread::tid()),
      looping_(false)
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << tid_;
    if (t_loop_in_this_thread)
    {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << tid_;
    }
    else
    {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::loop()
{
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;

    ::poll(NULL, 0, 500);

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::AbortNotInLoopThread()
{
    if (!IsInLoopThread())
    {
        LOG_FATAL << "This loop belongs to thread " << tid_ 
                  << " instead of current thread " << current_thread::tid();
    }
}

EventLoop *EventLoop::GetLoopOfCurrentThread()
{
    return t_loop_in_this_thread;
}

}   // namespace muzi
