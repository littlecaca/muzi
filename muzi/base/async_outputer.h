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

namespace muzi
{
class AsyncOutpter : public Outputer
{
public:
    void Output(const Outputer::Buffer &buffer) override;

    void Flush() override;

    void Start();

    void Stop();

private:
    void Run();

private:
    // not thread safe
    class LogFile : muzi::noncopyable
    {
    public:
        LogFile(int flush_interval, const std::string base_name, off_t roll_size)
            : flush_interval_(flush_interval), base_name_(base_name), roll_size_(roll_size) 
        { }

        void Append(const char *msg, size_t len);
        void Flush();
        bool RollFile();
        std::string GetFileName() const;

    private:
        const int flush_interval_;
        const std::string base_name_;
        const off_t roll_size_;
        
        time_t start_time_;
        time_t last_roll_;
        time_t last_flush_;

        std::ofstream os;

        const static int kRollPerSeconds = 60 * 60 * 24;
    };

    typedef FixedBuffer<config::kLargeBuffSize> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    LogFile log_file_;

    std::atomic<bool> running_;

    MutexLock mutex_;
    Condition cond_;
    CountdownLatch latch_;
    Thread thread_;

    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers_;
};
}   // namespace muzi

#endif  // MUZI_BASE_ASYN_LOG_H_
