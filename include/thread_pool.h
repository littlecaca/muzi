#ifndef MUZI_BASE_THREAD_POOL_H_
#define MUZI_BASE_THREAD_POOL_H_

#include "noncopyable.h"
#include "mutex.h"
#include "condition.h"
#include "thread.h"

#include <deque>
#include <functional>
#include <memory.h>


namespace muzi
{
class ThreadPool : noncopyable
{
public:
    typedef std::function<void(void)> Task;
    typedef std::vector<std::unique_ptr<Thread>> ThreadList;

    ThreadPool();
    ~ThreadPool();

    void SetMaxQueueSize(size_t size) { max_queue_size = size; }

    void SetThreadInitCallback(Task cb) { thread_init_callback_ = std::move(cb); }

    void Start(int thread_num);

    void Stop();

    /// @attention May block due to full queue.
    void Run(Task task);

private:
    bool IsFull() const;    // GUARDED_BY(lock_)

    void RunThread();

private:
    // defaults to -1, representing not queue size restrict
    int max_queue_size;

    MutexLock lock_;
    bool running_ GUARDED_BY(lock_);
    Condition not_full_cond_ GUARDED_BY(lock_);
    Condition not_empty_cond_ GUARDED_BY(lock_);
    std::deque<Task> queue_ GUARDED_BY(lock_);
    
    // the running of this callback_ has not thread-safe guarantee.
    Task thread_init_callback_;
    ThreadList thread_list_;
};

}   // namespace muzi

#endif  // MUZI_BASE_THREAD_POOL_H_
