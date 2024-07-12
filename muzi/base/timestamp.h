#ifndef MUZI_TIMESTAMP_H_
#define MUZI_TIMESTAMP_H_

#include <stdint.h>
#include <time.h>

#include "noncopyable.h"

namespace muzi
{
class TimeStamp : noncopyable
{
public:
    static struct timespec ToTimeSpec(int64_t usec)
    {
        struct timespec ts = {0, 0};
        ts.tv_sec = static_cast<time_t>(usec / kMicrosecondsPerSecond);
        ts.tv_nsec = static_cast<long>(usec % kMicrosecondsPerSecond * 1000);
        return ts;
    }

private:
    static constexpr int kMicrosecondsPerSecond = 1000 * 1000;
};
    
}


#endif  // MUZI_TIMESTAMP_H_