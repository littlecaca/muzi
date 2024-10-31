/**
 * Mutithread asyncronization logging module
 */
#ifndef MUZI_BASE_ASYN_LOG_H_
#define MUZI_BASE_ASYN_LOG_H_

#include <time.h>
#include <atomic>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "condition.h"
#include "config.h"
#include "countdown_latch.h"
#include "fixed_buffer.h"
#include "mutex.h"
#include "noncopyable.h"
#include "outputer.h"
#include "thread.h"
#include "log_file.h"

namespace muzi
{
class AsyncOutputer : public Outputer
{
public:
    AsyncOutputer(const std::string &base_name);

    void Output(const Outputer::SmallBuffer &buffer) override;

    void Start();

    void Stop();

    void Flush() override;

    ~AsyncOutputer() override
    {
        if (running_)
            Stop();
    }

private:
    void Run();

private:
    typedef FixedBuffer<config::kLargeBuffSize> LargeBuffer;
    typedef std::vector<std::unique_ptr<LargeBuffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    LogFile log_file_;
    int flush_interval_;

    std::atomic<bool> running_;

    MutexLock mutex_;
    Condition cond_ GUARDED_BY(mutex_);
    CountdownLatch latch_ GUARDED_BY(mutex_);
    Thread thread_;

    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers_;
};

extern AsyncOutputer *gAsyncOutputer;

}   // namespace muzi

#endif  // MUZI_BASE_ASYN_LOG_H_
