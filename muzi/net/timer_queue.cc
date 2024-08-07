#include "timer_queue.h"

namespace muzi
{
// void TimerQueue::AddTimer()
// {
// }

TimerQueue::TimerQueue(EventLoop *loop)
{
}

TimerQueue::~TimerQueue()
{
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval)
{
    return TimerId();
}

void TimerQueue::Cancel(TimerId timer_id)
{
}

} // namespace muzi
