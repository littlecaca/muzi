#ifndef MUZI_THREAD_H_
#define MUZI_THREAD_H_

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
}   // namespace thread
}   // namespace muzi


#endif