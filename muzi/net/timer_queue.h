#ifndef MUZI_NET_TIMER_QUEUE_H_
#define MUZI_NET_TIMER_QUEUE_H_

#include <memory>
#include <set>
#include <map>
#include <vector>

#include "event_loop.h"
#include "noncopyable.h"
#include "timer.h";
#include "timestamp.h"


namespace muzi
{
class TimerQueue : noncopyable
{
public:
    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    TimerId AddTimer(TimerCallback cb,
                     Timestamp when,
                     double interval);
    
    void Cancel(TimerId timer_id);

private:
    typedef std::pair<Timestamp, std::unique_ptr<Timer *>> Entry;
    typedef std::set<Entry> TimerSet;
    typedef std::map<TimerId, Timer *> ActiveTimerMap;

    void AddTimerInLoop(Timer *);
    
    void CancelTimerInLoop(Timer *);

    void HandleRead();

    std::vector<Entry> GetExpired(Timestamp now);
    



};

}   // namespace muzi

#endif  // MUZI_NET_TIMER_QUEUE_H_
