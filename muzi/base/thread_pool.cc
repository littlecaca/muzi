#include "thread_pool.h"

#include <exception>

#include "logger.h"
#include "traced_exception.h"

namespace muzi
{
ThreadPool::ThreadPool()
    : max_queue_size(-1),
      running_(false),
      not_full_cond_(lock_),
      not_empty_cond_(lock_) 
{
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::Start(int thread_num)
{
    if (running_) return;
    running_ = true;
    for (int i = 0; i < thread_num; ++i)
    {
        thread_list_.emplace_back(new Thread(std::bind(&ThreadPool::RunThread, this)));
        thread_list_.back()->Start();
    }
}

void ThreadPool::Stop()
{
    if (!running_) return;
    running_ = false;
    not_empty_cond_.NotifyAll();
    not_full_cond_.NotifyAll();
    for (auto &thread : thread_list_)
    {
        thread->Join();
    }
}

void ThreadPool::Run(Task task)
{
    if (!running_)
    {
        LOG_ERROR << "The thread poll has not been started";
        return;
    }

    if (!task)
    {
        LOG_ERROR << "Invalid task";
        return;
    }

    {
        // Push one task
        MutexLockGuard guard(lock_);
        while (IsFull())
        {
            not_full_cond_.Wait();
            if (!running_) return;
        }
        queue_.push_back(std::move(task));
    }
    not_empty_cond_.Notify();
}

bool ThreadPool::IsFull() const
{
    return (max_queue_size > 0 && queue_.size() >= max_queue_size);
}

void ThreadPool::RunThread()
{
    if (thread_init_callback_)
    {
        thread_init_callback_();
    }

    while (running_)
    {
        Task task;
        {
            // take one task
            MutexLockGuard guard(lock_);
            while (queue_.empty())
            {
                not_empty_cond_.Wait();
                if (!running_) return;
            }
            task = std::move(queue_.front());
            queue_.pop_front();
        }
        not_full_cond_.Notify();

        try
        {
            task();
        }
        catch (Exception &e)
        {
            LOG_ERROR << "muzi Exception caught in thread: " << current_thread::tid()
                      << "\nReason: " << e.what() 
                      << "\nTrace stack: " << e.GetStackTrace();
        }
        catch (std::exception &e)
        {
            LOG_ERROR << "Exception caught in thread: " << current_thread::tid()
                      << "\nReason: " << e.what();
        }
        catch (...)
        {
            LOG_ERROR << "Unknown exception caught in thread: " << current_thread::tid();
            throw;   
        }
    }
}

}   // namespace muzi
