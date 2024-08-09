#include "event_loop_thread.h"

namespace muzi
{
EventLoopThread::EventLoopThread(const std::string &name) 
    : thread_(std::bind(&EventLoopThread::ThreadFunc, this), name),
      cond_(lock_),
      loop_(nullptr)
{ }

EventLoopThread::~EventLoopThread()
{
    MutexLockGuard guard(lock_);
    if (loop_ != nullptr)
    {
        loop_->Quit();
        while (loop_ != nullptr)
            cond_.Wait();
    }
}

EventLoop *EventLoopThread::StartLoop()
{
    assert(loop_ == nullptr);

    if (thread_.IsStarted())
    {
        LOG_ERROR << "Thread has been started";
    }
    thread_.Start();

    EventLoop *loop = nullptr;
    // Wait thread to create EventLoop
    {
        MutexLockGuard guard(lock_);
        while (loop_ == nullptr)
        {
            cond_.Wait();
        }
        loop = loop_;
    }

    LOG_TRACE << "Start loop " << thread_.GetName();
    
    return loop;
}

bool EventLoopThread::IsLoopAlive()
{
    MutexLockGuard guard(lock_);
    return loop_ != nullptr;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;

    {
        MutexLockGuard guard(lock_);
        loop_ = &loop;
        cond_.Notify();
    }

    loop.Loop();

    {
        MutexLockGuard guard(lock_);
        loop_ = nullptr;
        cond_.Notify();
    }
}

} // namespace muzi
