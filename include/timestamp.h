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

class Timestamp
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
    Timestamp() 
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        time_val_ = tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec;
    }

    // Set other time
    explicit Timestamp(int64_t time) : time_val_(time)
    { }

    Timestamp(const struct timeval &tv)
        : time_val_(tv.tv_sec * kMicrosecondsPerSecond + tv.tv_usec)
    { }

    Timestamp(const Timestamp &ts) : time_val_(ts.time_val_)
    { }

    // not thread safe, read it out immediately
    StringProxy ToFormatString() const;

    std::string GetOriginalString() const
    {
        std::ostringstream os;
        os << "timeval(" << time_val_ / kMicrosecondsPerSecond << ", " 
           << time_val_ % kMicrosecondsPerSecond << ")";
        return os.str();
    }

    suseconds_t GetUsecs() const
    {
        return time_val_ % kMicrosecondsPerSecond;
    }

    std::time_t GetSecs() const
    {
        return time_val_ / kMicrosecondsPerSecond;
    }

    std::time_t GetLocalTime() const
    {
        return kLocalTimeZone.Convert(time_val_ / kMicrosecondsPerSecond);
    }

    std::time_t GetUtcTime() const
    {
        return kUtcTimeZone.Convert(time_val_ / kMicrosecondsPerSecond);
    }

    // Be careful! it is not thread safe
    static void SetTimeZone(const TimeZone *zone)
    {
        zone_validator_ = zone;
    }

    struct timeval GetTimeval() const 
    { 
        return {time_val_ / kMicrosecondsPerSecond, time_val_ % kMicrosecondsPerSecond}; 
    }

    
    int64_t ToUsecs() const { return time_val_; }

    static const TimeZone *GetTimeZone() { return zone_validator_; }
    
    Timestamp &operator-=(const Timestamp &rhs)
    {
        time_val_ -= rhs.time_val_;
        return *this;
    }

    Timestamp operator-(const Timestamp &rhs) const
    {
        return Timestamp(*this) -= rhs;
    }

    Timestamp &operator+=(const Timestamp &rhs)
    {
        time_val_ += rhs.time_val_;
        return *this;
    }

    Timestamp operator+(const Timestamp &rhs)
    {
        return Timestamp(*this) += rhs;
    }

    bool operator<(const Timestamp &rhs) const
    {   
        return time_val_ < rhs.time_val_;
    }

    bool operator>(const Timestamp &rhs) const
    {
        return time_val_ > rhs.time_val_;
    }

    bool operator==(const Timestamp &rhs) const
    {
        return time_val_ == rhs.time_val_;
    }

    Timestamp &AddTime(double interval_secs);

    void SetValid()
    {
        time_val_ = -1;
    }

    bool IsValid() const { return time_val_ > 0; }

    static Timestamp GetInvalid() { return Timestamp(-1); }
    
private:
    // Always points to local time
    int64_t time_val_;

private:
    const static TimeZone *zone_validator_;
};

}

#endif  // MUZI_TIMESTAMP_H_
