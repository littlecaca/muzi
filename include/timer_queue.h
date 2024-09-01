#ifndef MUZI_NET_TIMER_QUEUE_H_
#define MUZI_NET_TIMER_QUEUE_H_

#include <memory>
#include <set>
#include <map>
#include <unordered_set>
#include <vector>

#include "event_loop.h"
#include "channel.h"
#include "noncopyable.h"
#include "timer.h"
#include "timestamp.h"

namespace muzi
{
class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    /// @brief An uniform interface to add timers in loop thread or not
    /// @return TimerId is a incremental number to uniquely mark a timer
    TimerId AddTimer(const TimerCallback &cb,
                     Timestamp when,
                     double interval);

    /// @brief An uniform interface to cancel timers in loop thread or not
    void Cancel(TimerId timer_id);

private:
    // Here we can use heterogeneous comparison to find an Entry in a TimerSet
    typedef std::unique_ptr<Timer> Entry;

    // It uses the feature named "heterogeneous comparison" from C++14
    struct CompareEntry
    {
        typedef void is_transparent;    // Enable heterogeneous lookup

        /// @brief Plain comparison
        bool operator()(const Entry &lhs, const Entry &rhs) const
        {
            return lhs->GetExpiration() < rhs->GetExpiration()
                || (lhs->GetExpiration() == rhs->GetExpiration() && lhs.get() < rhs.get());
        }
        
        /// @brief Heterogeneous comparison
        bool operator()(const Entry &lhs, const Timer *rhs) const
        {
            return lhs->GetExpiration() < rhs->GetExpiration()
                || (lhs->GetExpiration() == rhs->GetExpiration() && lhs.get() < rhs);
        }

        /// @brief Heterogeneous comparison
        bool operator()(const Timer *lhs, const Entry &rhs) const
        {
            return lhs->GetExpiration() < rhs->GetExpiration()
                || (lhs->GetExpiration() == rhs->GetExpiration() && lhs < rhs.get());
        }

        /// @brief Heterogeneous comparison
        bool operator()(const Entry &lhs, Timestamp when) const
        {
            return lhs->GetExpiration() < when;
        }

        /// @brief Heterogeneous comparison
        bool operator()(Timestamp when, const Entry &lhs) const
        {
            return when < lhs->GetExpiration();
        }
    };

    typedef std::set<Entry, CompareEntry> TimerSet;
    typedef std::vector<Entry> TimerList;
    typedef std::unordered_set<TimerId> TimerIdSet;

    // When cancel timer by TimerId, we do not know whether the
    // timer has already been deleted. If it has been deleted, 
    // we can not get the Timer *'s expiration to find it in timers_.
    // That is why we need this ActiveTimerMap.
    typedef std::map<TimerId, Timer *> ActiveTimerMap;

private:
    void AddTimerInLoop(Timer *timer);
    
    void CancelTimerInLoop(const TimerId &timer_id);

    void HandleRead();

    void GetExpired(Timestamp now);

    void ResetTimer(Timestamp now);
    
    /// @brief Insert timer to this TimerQueue
    /// @attention Must be called in Loop thread. And each timer only can
    /// be inserted once.
    /// @return if timer's expiration is ealiest then return true
    bool Insert(Timer *timer);

private:
    EventLoop *loop_;
    const int timer_fd_;
    Channel timer_channel_;
    bool is_executing_timers_;  // atomic

    // Timers are stored in ascending order
    // The lifecycle of Timer depends on the timers_
    TimerSet timers_;

    // Store timers that are alive
    ActiveTimerMap active_timers_;

    // Store expired timers
    TimerList expired_timers_;

    // Store canceling timers_ which may be in the expired_timers_,
    // because Timer::Run() may cancel timers in which time they
    // have been deleted from active_timers_ and timers_. So we
    // must record it to avoid reseting the canceled timers.
    // Used by ResetExpired().
    TimerIdSet canceling_timers_;
};

}   // namespace muzi

#endif  // MUZI_NET_TIMER_QUEUE_H_
