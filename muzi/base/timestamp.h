#ifndef MUZI_TIMESTAMP_H_
#define MUZI_TIMESTAMP_H_

#include <stdint.h>
#include <ctime>
#include <sstream>

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

    // Default using now
    TimeStamp() 
    {
        gettimeofday(&time_val_, nullptr);
    }

    // Set other time
    TimeStamp(std::time_t time)
    {
        time_val_.tv_sec = time;
        time_val_.tv_usec = 0;
    }

    TimeStamp(const struct timeval &tv)
        : time_val_(tv)
    { }

    // not thread safe, read it out immediately
    StringProxy ToFormatString() const;

    std::string GetOriginalString() const
    {
        std::ostringstream os;
        os << "timeval(" << time_val_.tv_sec << ", " << time_val_.tv_usec << ")";
        return os.str();
    }

    suseconds_t GetUsecs() const
    {
        return time_val_.tv_sec * kMicrosecondsPerSecond + time_val_.tv_usec;
    }

    std::time_t GetSecs() const
    {
        return time_val_.tv_sec;
    }

    std::time_t GetLocalTime() const
    {
        return kLocalTimeZone.Convert(time_val_.tv_sec);
    }

    std::time_t GetUtcTime() const
    {
        return kUtcTimeZone.Convert(time_val_.tv_sec);
    }

    // Be careful! it is not thread safe
    static void SetTimeZone(const TimeZone *zone)
    {
        zone_validator_ = zone;
    }

    struct timeval GetTimeval() const { return time_val_; }

    static const TimeZone *GetTimeZone() { return zone_validator_; }
    
    TimeStamp operator-(const TimeStamp &rhs) const
    {
        return timeval{time_val_.tv_sec - rhs.GetTimeval().tv_sec, 
                time_val_.tv_usec - rhs.GetTimeval().tv_usec};
    }

private:
    // Always points to local time
    struct timeval time_val_ = {0};

private:
    const static TimeZone *zone_validator_;
};

}

#endif  // MUZI_TIMESTAMP_H_
