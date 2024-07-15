#ifndef MUZI_TIMESTAMP_H_
#define MUZI_TIMESTAMP_H_

#include <stdint.h>
#include <ctime>
#include <sys/time.h>

#include "noncopyable.h"
#include "string_proxy.h"
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

    explicit TimeStamp(const TimeZone &zone_validtor) 
        : zone_validtor_(zone_validtor)
    {
        gettimeofday(&time_val_, nullptr);
    }
    // Default using utc timezone
    TimeStamp() : TimeStamp(kUtcTimeZone) {}
    // Set other time
    TimeStamp(std::time_t time, const TimeZone &zone_validtor) : zone_validtor_(zone_validtor)
    {
        time_val_.tv_sec = time;
        time_val_.tv_usec = 0;
    }
    
    explicit TimeStamp(std::time_t time) : TimeStamp(time, kUtcTimeZone) {}

    // The returned string uses TLS, but it won't stop overwritting
    // So be careful
    StringProxy ToFormatString() const;

    std::time_t ToLocalTime() const
    {
        return kLocalTimeZone.Convert(time_val_.tv_sec);
    }

    std::time_t ToUtcTime() const
    {
        return kUtcTimeZone.Convert(time_val_.tv_sec);
    }

private:
    // Always points to local time
    struct timeval time_val_ = {0};
    const TimeZone &zone_validtor_;
};

}


#endif  // MUZI_TIMESTAMP_H_