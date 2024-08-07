#ifndef MUZI_NET_TIMER_H_
#define MUZI_NET_TIMER_H_

#include <atomic>
#include <functional>

#include "noncopyable.h"
#include "timestamp.h"

namespace muzi
{
class Timer : noncopyable
{
public:
    typedef std::function<void()> TimerCallback;
    Timer(TimerCallback callback, TimeStamp when, double interval)
        : cb_(std::move(callback)),
          expiration_(when),
          interval_(interval),
          repeated_(interval > 0.0),
          sequence_(s_num_created_.fetch_add(1))
    {
    }

    void Run() const 
    {
        cb_();
    }

    TimeStamp GetExpiration() const { return expiration_; }
    double GetInterval() const { return interval_; }
    bool IsRepeated() const { return repeated_; }
    int64_t GetSequence() const { return sequence_; }

    void Restart(TimeStamp now);

    void Refresh();

    static int64_t GetNumCreated() { return s_num_created_; }

private:
    TimerCallback cb_;
    TimeStamp expiration_;
    const double interval_;
    const bool repeated_;
    const int64_t sequence_;

    static std::atomic_int64_t s_num_created_;
};
}   // namespace muzi

#endif // MUZI_NET_TIMER_H_
