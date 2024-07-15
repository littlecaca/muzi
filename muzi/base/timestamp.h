#ifndef MUZI_TIMESTAMP_H_
#define MUZI_TIMESTAMP_H_

#include <stdint.h>
#include <ctime>

#include "noncopyable.h"
#include "timezone.h"

namespace muzi
{



class TimeStamp : noncopyable
{
public:
    static std::timespec ToTimeSpec(int64_t usec, std::timespec &ts)
    {
        ts.tv_sec = static_cast<time_t>(usec / kMicrosecondsPerSecond);
        ts.tv_nsec = static_cast<long>(usec % kMicrosecondsPerSecond * 1000);
        return ts;
    }
    
    static constexpr int kMillisecondsPerSecond = 1000;
    static constexpr int kMicrosecondsPerSecond = 1000 * 1000;
    static constexpr int kNanosecondsPerSecond = 1000 * 1000 * 1000;

    TimeStamp() : time_(std::time(nullptr)) {}
    TimeStamp(std::time_t time) : time_(time) {}


   

private:
    std::time_t time_;
};
    
}


#endif  // MUZI_TIMESTAMP_H_