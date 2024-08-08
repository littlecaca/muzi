#ifndef MUZI_NET_TIMER_H_
#define MUZI_NET_TIMER_H_

#include <atomic>
#include <functional>

#include "noncopyable.h"
#include "timestamp.h"
#include "logger.h"

namespace muzi
{
typedef int64_t TimerId;
typedef std::function<void()> TimerCallback;

class Timer : noncopyable
{
public:
    Timer(const TimerCallback &callback, Timestamp when, double interval)
        : cb_(callback),
          expiration_(when),
          interval_(interval),
          repeated_(interval > 0.0),
          sequence_(s_num_created_.fetch_add(1))
    {
        if (interval < 0)
        {
            LOG_ERROR << "interval must be positive";
        }
    }

    void Run() const 
    {
        cb_();
    }

    Timestamp GetExpiration() const { return expiration_; }
    double GetInterval() const { return interval_; }
    bool IsRepeated() const { return repeated_; }
    TimerId GetId() { return sequence_; }

    void Restart(Timestamp now);

    void Refresh();

    static int64_t GetNumCreated() { return s_num_created_; }

private:
    TimerCallback cb_;
    Timestamp expiration_;
    const double interval_;
    const bool repeated_;
    const int64_t sequence_;

    static std::atomic_int64_t s_num_created_;
};

}   // namespace muzi

#endif // MUZI_NET_TIMER_H_
