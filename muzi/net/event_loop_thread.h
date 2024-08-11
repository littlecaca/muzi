#ifndef MUZI_NET_EVENT_LOOP_THREAD_H_
#define MUZI_NET_EVENT_LOOP_THREAD_H_

#include <memory>
#include <string>

#include "condition.h"
#include "event_loop.h"
#include "mutex.h"
#include "noncopyable.h"
#include "thread.h"

namespace muzi
{
/// @attention When EventLoopThread is destroyed, it will quit the loop and wait
/// the thread to exit.
class EventLoopThread : noncopyable
{
public:
    typedef std::function<void(EventLoop *loop)> ThreadInitCallBack;

    EventLoopThread(const ThreadInitCallBack &init_cb, const std::string &name = "EventLoop");

    EventLoopThread(const std::string &name = "EventLoop")
        : EventLoopThread(nullptr, name)
    {
    }

    ~EventLoopThread();

    /// @brief Create a thread to run an EventLoop.
    /// It does not own the EventLoop. The EventLoop will be a
    /// stack object in the thread. But when it is destroyed it
    /// will try calling the loop->Quit().
    /// @return A pointer to the EventLoop. Pay attention that
    /// the pointer may be a dangling pointer.
    EventLoop *StartLoop();

    /// @brief Check if the EventLoop * is nullptr.
    bool IsLoopAlive();

private:
    void ThreadFunc();
    
private:
    Thread thread_;
    MutexLock lock_;
    Condition cond_ GUARDED_BY(lock_);    
    EventLoop *loop_ GUARDED_BY(lock_);
    ThreadInitCallBack init_cb_;
};

}   // namespace muzi

#endif  // MUZI_NET_EVENT_LOOP_THREAD_H_
