#include "timer_queue.h"

#include <algorithm>
#include <iterator>
#include <functional>
#include <sys/timerfd.h>
#include <unistd.h>

#include "logger.h"

namespace muzi
{
namespace
{
int CreateTimerFd()
{
    return ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
}

void ResetTimerFd(int timer_fd, Timestamp time)
{   
    struct itimerspec new_value;
    memset(&new_value, 0, sizeof new_value);
    time -= Timestamp();
    // If the timer has already expired or invalid
    // then better trigger it right now
    if (!time.IsValid())
        time = Timestamp(10);

    new_value.it_value.tv_sec = time.GetSecs();
    new_value.it_value.tv_nsec = static_cast<long>(time.GetUsecs() * 1000);

    if (::timerfd_settime(timer_fd, 0, &new_value, NULL))
    {
        LOG_SYSERR << "::timerfd_settime() fails";
    }
}

void ReadTimerFd(int timer_fd, Timestamp time)
{
    uint64_t how_many;
    ssize_t n = ::read(timer_fd, &how_many, sizeof how_many);
    LOG_TRACE << "TimerQueue::HandleRead() has handled " << how_many << " at " << time.ToFormatString();
    if (n != sizeof how_many)
    {
        LOG_ERROR << "ReadTimerFd() reads " << n << " bytes instead of " << sizeof how_many;
    }
}

}   // internal linkage

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timer_fd_(CreateTimerFd()),
      timer_channel_(loop, timer_fd_),
      is_executing_timers_(false)
{
    timer_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    timer_channel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    timer_channel_.DisableAll();
    timer_channel_.Remove();
    ::close(timer_fd_);
}

TimerId TimerQueue::AddTimer(const TimerCallback& cb, Timestamp when, double interval)
{
    if (!when.IsValid())
    {
        LOG_FATAL << "when is invalid";
    }

    Timer *timer = new Timer(cb, when, interval);
    TimerId id = timer->GetId();
    if (loop_->IsInLoopThread())
    {
        AddTimerInLoop(timer);
    }
    else
    {
        loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    }
    return id;
}

void TimerQueue::Cancel(TimerId timer_id)
{
    if (loop_->IsInLoopThread())
    {
        CancelTimerInLoop(timer_id);
    }
    else
    {
        loop_->RunInLoop(std::bind(&TimerQueue::CancelTimerInLoop, this, timer_id));
    }
}

void TimerQueue::AddTimerInLoop(Timer *timer)
{
    loop_->AssertInLoopThread();
    bool earliest = Insert(timer);
    if (earliest)
    {
        ResetTimerFd(timer_fd_, timer->GetExpiration());
    }
}

void TimerQueue::CancelTimerInLoop(const TimerId &timer_id)
{
    loop_->AssertInLoopThread();
    
    auto it = active_timers_.find(timer_id);
    if (it != active_timers_.end())
    {
        // Delete the timer in active_timers_ and timers_
        auto timer_iter = timers_.find(it->second); // Heterogeneous comparison
        assert(timer_iter != timers_.end());
        active_timers_.erase(it);
        timers_.erase(timer_iter);
    }
    else if (is_executing_timers_)
    {
        // If the timer has already be deleted,
        // it may be stored in expired_timers_.
        // If this function is called by Timer::Run(),
        // we need to make sure the timer will not repeat again.
        canceling_timers_.insert(timer_id);
    }

    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead()
{
    loop_->AssertInLoopThread();

    Timestamp now;
    // Must read timerfd to stop busy loop in "Level Trigger" mode
    ReadTimerFd(timer_fd_, now);
    GetExpired(now);
    // Run timer callback
    is_executing_timers_ = true;
    for (Entry &timer : expired_timers_)
    {
        timer->Run();
    }
    is_executing_timers_ = false;

    ResetTimer(now);
    
    assert(canceling_timers_.empty());
    assert(expired_timers_.empty());
}

void TimerQueue::GetExpired(Timestamp now)
{
    // Find the expired timers
    auto end = timers_.upper_bound(now);   // Heterogeneous comparison
    assert(end == timers_.end() || (*end)->GetExpiration() > now);

    // Erase expired timers in timers_ and active_timers
    assert(expired_timers_.empty());
    for (auto it = timers_.begin(); it != end; ++it)
    {
        // Attend the order
        active_timers_.erase((*it)->GetId());

        // The key itertor in all associative containers is const T &
        // So here we use const_cast to move it. It's OK as long as
        // we erase it immediately.
        expired_timers_.push_back(std::move(const_cast<Entry &>(*it)));
    }
    timers_.erase(timers_.begin(), end);
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::ResetTimer(Timestamp now)
{
    // Reset repeated timer
    for (Entry &timer : expired_timers_)
    {
        if (timer->IsRepeated()
            && canceling_timers_.find(timer->GetId()) == canceling_timers_.end())
        {
            timer->Restart(now);
            // Transfer ownership of Timer
            Insert(timer.get());
            timer.release();
        }
    }

    expired_timers_.clear();
    canceling_timers_.clear();

    // Reset timer_fd_
    if (!timers_.empty())
    {
        Timestamp next_expired = (*timers_.begin())->GetExpiration();
        if (next_expired.IsValid())
        {
            ResetTimerFd(timer_fd_, next_expired);
        }
    }
}

bool TimerQueue::Insert(Timer *timer)
{
    bool earliest = timers_.empty() || timer->GetExpiration() < (*timers_.begin())->GetExpiration();

    // Insert timer to timers_
    {
        auto res = timers_.insert(Entry(timer));
        assert(res.second); (void)res;
    }

    // Insert timer to active_timers_
    {
        auto res = active_timers_.insert({timer->GetId(), timer});
        assert(res.second); (void)res;
    }

    assert(timers_.size() == active_timers_.size());

    return earliest;
}

} // namespace muzi
