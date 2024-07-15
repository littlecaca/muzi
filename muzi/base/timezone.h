#ifndef MUZI_BASE_TIMEZONE_H_
#define MUZI_BASE_TIMEZONE_H_

#include <ctime>

#include "noncopyable.h"

namespace muzi
{
class TimeZone : noncopyable
{
public:
    virtual std::tm *Convert(const std::time_t time) const = 0;
};

class LocalTimeZone : public TimeZone
{
public:
    virtual std::tm *Convert(const std::time_t time) const override
    {
        return std::localtime(&time);
    }
};

class UtcTimeZone : public TimeZone
{
public:
    std::tm *Convert(const std::time_t time) const override
    {
        return std::gmtime(&time);
    }
};

}   // namespace muzi

#endif  // MUZI_BASE_TIMEZONE_H_