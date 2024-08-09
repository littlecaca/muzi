#include "thread.h"

#include <stdlib.h>
#include <atomic>
#include <exception>
#include <iostream>
#include <memory>
#include <utility>

#include "current_thread.h"
#include "logger.h"

namespace muzi
{
namespace
{
struct ThreadData
{
    Thread::ThreadFunc func_;
    pid_t *tid_;
    CountdownLatch *latch_;
    std::string name_;
};

std::atomic<int> thread_num = 0;

}   // internal linkage

void Thread::SetDefaultName()
{
    int num = thread_num.fetch_add(1);
    if (name_.empty())
        name_ = "Thread" + std::to_string(num);
}

void Thread::Join()
{
    assert(started_ && !joined_);
    pthread_join(pthread_id_, NULL);
    joined_ = true;
}

void *RunThread(void *args)
{
    std::unique_ptr<ThreadData> data(static_cast<ThreadData *>(args));
    *data->tid_ = current_thread::tid();
    data->tid_ = NULL;
    // Free the parent thread
    data->latch_->CountDown();
    data->latch_ = NULL;
    
    try
    {
        data->func_();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception caught in thread: " << data->name_ << "\n";
        std::cerr << "Reason: " << e.what() << std::endl;
        abort();
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught in thread: " << data->name_ << std::endl;
        throw;   
    }
    return NULL;
}

void Thread::Start()
{
    assert(!started_);
    started_ = true;
    
    ThreadData *data = new ThreadData{func_, &tid_, &latch_, name_};
    if (::pthread_create(&pthread_id_, NULL, RunThread, data))
    {
        // pthread_create() fails
        started_ = false;
        delete data;
        LOG_SYSERR << "Failed in pthread_create";
    }
    else
        latch_.Wait();
}

}   // namespace muzi
