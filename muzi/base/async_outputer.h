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
#include "file_utils.h"
#include "fixed_buffer.h"
#include "mutex.h"
#include "noncopyable.h"
#include "outputer.h"
#include "thread.h"

namespace muzi
{
class AsyncOutputer : public Outputer
{
public:
    AsyncOutputer(const std::string &base_name);

    void Output(const Outputer::Buffer &buffer) override;

    void Start();

    void Stop();

    ~AsyncOutputer() override
    {
        if (running_)
            Stop();
    }

private:
    void Run();

private:
    // not thread safe
    class LogFile : muzi::noncopyable
    {
    public:
        LogFile(const std::string base_name)
            : written_bytes_(0), last_roll_(0),
              last_flush_(0), count_(0)
        { 
            SetBaseName(base_name);
            RollFile();
        }

        // Unlocked writting
        void Append(const char *msg, size_t len);
        // Unlocked flushing
        void Flush();
        bool RollFile();

    private:
        void SetBaseName(const std::string &base_name);
        std::string base_name_;

        off_t written_bytes_;
        time_t last_roll_;
        time_t last_flush_;

        file_utils::UnlockedWriteFile file_;

        int count_;

        const static off_t kRollSize = config::kRollSize * 1024 * 1024;
        const static int kCheckEveryN = 1024;
        const static int kFlushIntervalSeconds = 3;
        const static int kRollIntervalSeconds = 60 * 60 * 24;
    };

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
