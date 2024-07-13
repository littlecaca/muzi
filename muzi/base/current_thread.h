/**
 * current_thread is a namespace storing four TLS objects
 */
#ifndef MUZI_BASE_CURRENTTHREAD_H_
#define MUZI_BASE_CURRENTTHREAD_H_

#include <stdint.h>
#include <string>

#include "macros.h"

namespace muzi {
namespace current_thread
{

// thread local storage
extern thread_local pid_t t_tid;
extern thread_local std::string t_tid_string;
extern thread_local int t_tid_length;
extern thread_local std::string t_thread_name;

void CachedTid();
bool IsMainThread();
void SleepUsec(int64_t usec);

inline pid_t tid()
{
    if (LIKELY(t_tid == 0))
    {
        CachedTid();
    }
    return t_tid;
}

inline const std::string &tid_string()
{
    return t_tid_string;
}

inline int tid_length()
{
    return t_tid_length;
}

inline const std::string &thread_name()
{
    return t_thread_name;
}

}   // namespace current_thread
}   // namespace muzi

#endif  // MUZI_BASE_CURRENTTHREAD_H_