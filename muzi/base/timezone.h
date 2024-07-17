#ifndef MUZI_BASE_TIMEZONE_H_
#define MUZI_BASE_TIMEZONE_H_

#include <time.h>
#include <ctime>

#include <sys/time.h>

#include "noncopyable.h"

namespace muzi
{
class TimeZone : noncopyable
{
public:
    virtual void Convert(std::time_t time, std::tm *time_buf) const = 0;
    virtual std::time_t Convert(std::time_t time) const = 0;
    virtual const char *GetTzName() const = 0;
};

class LocalTimeZone : public TimeZone
{
public:
    std::time_t Convert(std::time_t time) const
    {
        return time;
    }

    void Convert(std::time_t time, std::tm *time_buf) const override
    {
        localtime_r(&time, time_buf);
    }

    const char *GetTzName() const override
    {
        return tzname[0];
    }
};

class UtcTimeZone : public TimeZone
{
public:
    std::time_t Convert(std::time_t time) const
    {
        return time - timezone;
    }

    void Convert(std::time_t time, std::tm *time_buf) const override
    {
        gmtime_r(&time, time_buf);
    }

    const char *GetTzName() const override
    {
        return "UTC";
    }
};

// Always be used as const static object
extern const LocalTimeZone kLocalTimeZone;
extern const UtcTimeZone kUtcTimeZone;

}   // namespace muzi

#endif  // MUZI_BASE_TIMEZONE_H_
