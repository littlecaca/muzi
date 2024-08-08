#include "timestamp.h"

#include <ctime>

#include "logger.h"

namespace muzi
{

namespace
{
thread_local char t_fmt_buf[64];
thread_local std::tm t_time_buf;
thread_local time_t last_second = 0;
}   // internal linkage

const TimeZone *Timestamp::zone_validator_ = &kUtcTimeZone;

StringProxy Timestamp::ToFormatString() const 
{
    // For good performance
    std::time_t sec = GetSecs();
    if (sec != last_second)
    {
        zone_validator_->Convert(sec, &t_time_buf);
        last_second = sec;
    }
    
    snprintf(t_fmt_buf, sizeof t_fmt_buf, "%4d%02d%02d %02d:%02d:%02d.%06ld", 
        t_time_buf.tm_year + 1900, t_time_buf.tm_mon + 1, t_time_buf.tm_mday, t_time_buf.tm_hour,
        t_time_buf.tm_min, t_time_buf.tm_sec, static_cast<long>(GetUsecs()));
    return {t_fmt_buf, 24};
}

Timestamp &Timestamp::AddTime(double interval_secs)
{
    time_val_ += static_cast<int64_t>(interval_secs * kMicrosecondsPerSecond);
    return *this;
}

}   // namespace muzi
