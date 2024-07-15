#include "timestamp.h"

#include <ctime>

namespace muzi
{
thread_local char fmt_buf[64];
thread_local std::tm time_buf;

StringProxy TimeStamp::ToFormatString() const 
{
    zone_validtor_.Convert(time_val_.tv_sec, &time_buf);

    snprintf(fmt_buf, sizeof fmt_buf, "%4d%02d%02d %02d:%02d:%02d.%06ld", 
        time_buf.tm_year + 1900, time_buf.tm_mon + 1, time_buf.tm_mday, time_buf.tm_hour,
        time_buf.tm_min, time_buf.tm_sec, static_cast<long>(time_val_.tv_usec));
    return {fmt_buf, 24};
}

}   // namespace muzi