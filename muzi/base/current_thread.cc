#include "current_thread.h"

#include <time.h>

#include <unistd.h>

#include "timestamp.h"
#include "thread.h"

namespace muzi::current_thread
{
namespace
{
// thread local storage
thread_local pid_t t_tid = 0;
thread_local std::string t_tid_string;
thread_local std::string t_thread_name = "default";

void CachedTid()
{
    if (t_tid == 0)
    {
        t_tid = muzi::thread::GetTid();
        t_tid_string = std::to_string(t_tid);
    }
}; 
}   // internal linkage

// if pid_t is not integral, then probably it cannot be converted to string
static_assert(std::is_integral<pid_t>::value, "pid_t should be integral");

bool IsMainThread()
{
    return tid() == getpid();
}

void SleepUsec(int64_t usec)
{
    struct timespec ts = TimeStamp::ToTimeSpec(usec);
    ::nanosleep(&ts, NULL);
}

}   // namespace muzi
