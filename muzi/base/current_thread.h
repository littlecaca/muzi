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
void ResetTid();
extern thread_local pid_t t_tid;
extern thread_local std::string t_tid_string;

bool IsMainThread();
void SleepUsec(int64_t usec);

inline pid_t tid()
{
    if (UNLIKELY(t_tid == 0))
    {
        ResetTid();
    }
    return t_tid;
}

inline const std::string &tid_string()
{
    if (UNLIKELY(t_tid == 0))
    {
        ResetTid();
    }
    return t_tid_string;
}

}   // namespace current_thread
}   // namespace muzi

#endif  // MUZI_BASE_CURRENTTHREAD_H_
