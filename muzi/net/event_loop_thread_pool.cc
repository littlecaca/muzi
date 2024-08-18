#include "event_loop_thread_pool.h"

namespace muzi
{
EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::Start(ThreadInitCallback callback)
{
    base_loop_->AssertInLoopThread();
    assert(!started_);
    
    for (int i = 0; i < thread_nums_; ++i)
    {
        threads_.push_back(std::make_unique<EventLoopThread>
            (std::move(callback), name_ + std::to_string(i)));

        event_loops_.push_back(threads_.back()->StartLoop());
    }
    if (thread_nums_ == 0 && callback)
    {
        callback(base_loop_);
    }
    started_ = true;
}

EventLoop *EventLoopThreadPool::GetNextLoop()
{
    base_loop_->AssertInLoopThread();
    assert(started_);

    EventLoop *loop = base_loop_;

    if (!event_loops_.empty())
    {
        loop = event_loops_[next_];
        next_ = (next_ + 1) % event_loops_.size();
    }
    return loop;
}

EventLoop *EventLoopThreadPool::GetLoopForHash(size_t hashcode)
{
    base_loop_->AssertInLoopThread();
    assert(started_);
    
    EventLoop *loop = base_loop_;
    if (!event_loops_.empty())
    {
        loop = event_loops_[hashcode % event_loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::GetAllLoops() const
{
    base_loop_->AssertInLoopThread();
    assert(started_);

    if (!event_loops_.empty())
    {
        return event_loops_;
    }
    else
    {
        return std::vector<EventLoop *>(1, base_loop_);
    }
}

}   // namespace muzi
