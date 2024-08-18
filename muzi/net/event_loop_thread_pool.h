#ifndef MUZI_NET_EVENT_LOOP_THREAD_POOL_H_
#define MUZI_NET_EVENT_LOOP_THREAD_POOL_H_

#include <string>
#include <vector>

#include "event_loop.h"
#include "event_loop_thread.h"
#include "noncopyable.h"

namespace muzi
{
class EventLoopThreadPool : noncopyable
{
public:
    typedef EventLoopThread::ThreadInitCallback ThreadInitCallback;

    EventLoopThreadPool(EventLoop *base_loop, const std::string &name)
        : base_loop_(base_loop), 
          name_(name), 
          thread_nums_(0), 
          next_(0), 
          started_(false)
    {
    }

    ~EventLoopThreadPool();

    void SetThreadNum(int num) { thread_nums_ = num; }

    void Start(ThreadInitCallback callback = ThreadInitCallback());

    EventLoop *GetNextLoop();

    EventLoop *GetLoopForHash(size_t hashcode);

    std::vector<EventLoop *> GetAllLoops() const;

    bool IsStarted() const { return started_; }

    const std::string &GetName() const { return name_; }

private:
    EventLoop *base_loop_;
    std::string name_;
    int thread_nums_;
    int next_;
    bool started_;

    std::vector<EventLoop *> event_loops_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
};

}   // namespace muzi

#endif  // MUZI_NET_EVENT_LOOP_THREAD_POOL_H_
