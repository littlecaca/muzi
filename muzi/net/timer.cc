#include "timer.h"

namespace muzi
{
std::atomic_int64_t Timer::s_num_created_ = 0;

void Timer::Restart(Timestamp now)
{
    expiration_ = now.AddTime(interval_);
}

void Timer::Refresh()
{
    expiration_.AddTime(interval_);
}

} // namespace muzi
