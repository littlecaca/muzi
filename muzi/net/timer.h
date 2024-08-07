#ifndef MUZI_NET_TIMER_H_
#define MUZI_NET_TIMER_H_

#include <atomic>
#include <functional>

#include "noncopyable.h"
#include "timestamp.h"

namespace muzi
{
typedef int64_t TimerId;
typedef std::function<void()> TimerCallback;

class Timer : noncopyable
{
public:
    Timer(TimerCallback callback, Timestamp when, double interval)
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

    Timestamp GetExpiration() const { return expiration_; }
    double GetInterval() const { return interval_; }
    bool IsRepeated() const { return repeated_; }
    TimerId GetSequence() const { return sequence_; }

    void Restart(Timestamp now);

    void Refresh();

    static TimerId GetNumCreated() { return s_num_created_; }

private:
    TimerCallback cb_;
    Timestamp expiration_;
    const double interval_;
    const bool repeated_;
    const TimerId sequence_;

    static std::atomic_int64_t s_num_created_;
};


}   // namespace muzi

#endif // MUZI_NET_TIMER_H_
