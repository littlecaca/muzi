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
    virtual void Convert(struct timeval *tv) const = 0;
    virtual const char *GetTzName() const = 0;
};

class LocalTimeZone : public TimeZone
{
public:
    void Convert(struct timeval *tv) const override
    {
        // Do nothing
    }

    const char *GetTzName() const override
    {
        return tzname[0];
    }
};

class UtcTimeZone : public TimeZone
{
public:
    virtual void Convert(struct timeval *tv) const override 
    {
        tv->tv_sec -= timezone;
    }

    const char *GetTzName() const override
    {
        return "UTC";
    }
};

// Always be used as const static object
extern const TimeZone &kLocalTimeZone;
extern const TimeZone &kUtcTimeZone;

}   // namespace muzi

#endif  // MUZI_BASE_TIMEZONE_H_