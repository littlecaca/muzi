#include "timestamp.h"

#include <ctime>

namespace muzi
{

namespace
{
thread_local char t_fmt_buf[64];
thread_local std::tm t_time_buf;
}   // internal linkage

StringProxy TimeStamp::ToFormatString() const 
{
    zone_validtor_.Convert(time_val_.tv_sec, &t_time_buf);

    snprintf(t_fmt_buf, sizeof t_fmt_buf, "%4d%02d%02d %02d:%02d:%02d.%06ld", 
        t_time_buf.tm_year + 1900, t_time_buf.tm_mon + 1, t_time_buf.tm_mday, t_time_buf.tm_hour,
        t_time_buf.tm_min, t_time_buf.tm_sec, static_cast<long>(time_val_.tv_usec));
    return {t_fmt_buf, 24};
}

}   // namespace muzi