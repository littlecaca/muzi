#ifndef MUZI_BASE_THREAD_H_
#define MUZI_BASE_THREAD_H_

#include <functional>
#include <thread>
#include <string>

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "countdown_latch.h"

namespace muzi
{
namespace thread
{
inline pid_t GetTid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}
}   // namespace thread

class Thread
{
public:
    typedef std::function<void()> ThreadFunc;

    Thread(ThreadFunc func, const std::string &name) : func_(func), started_(false),
        joined_(false), name_(name), latch_(1) 
    {
        SetDefaultName();   
    }

    ~Thread()
    {
        if (started_ && !joined_)
            pthread_detach(pthread_id_);
    }

    void Start();

    void Join();

    bool IsStarted() const { return started_; }

    bool IsJoined() const { return joined_; }

    pthread_t GetPthreadId() const { return pthread_id_; }

    pid_t GetTid() const { return tid_; }
    
    std::string GetName() const { return name_; }
    
private:
    void SetDefaultName();

private:
    ThreadFunc func_;
    bool started_;
    bool joined_;
    pthread_t pthread_id_;
    pid_t tid_;
    std::string name_;
    CountdownLatch latch_;
};
}   // namespace muzi

#endif  // MUZI_BASE_THREAD_H_
