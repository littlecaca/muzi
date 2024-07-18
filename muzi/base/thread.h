#ifndef MUZI_BASE_THREAD_H_
#define MUZI_BASE_THREAD_H_

#include <functional>
#include <thread>
#include <string>

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace muzi
{
namespace thread
{
inline pid_t GetTid()
{
    return static_cast<pid_t>(::syscall(SYS_gettid));
}

class Thread
{
public:
    typedef std::function<void()> ThreadFunc;

    void start();

private:
    ThreadFunc func_;
    bool started_;
    bool joined_;
    pthread_t pthread_id_;
    pid_t tid_;
    std::string name_;

};


}   // namespace thread
}   // namespace muzi


#endif  // MUZI_BASE_THREAD_H_
