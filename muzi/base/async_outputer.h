/**
 * Mutithread asyncronization logging module
 */
#ifndef MUZI_BASE_ASYN_LOG_H_
#define MUZI_BASE_ASYN_LOG_H_


#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "condition.h"
#include "config.h"
#include "countdown_latch.h"
#include "fixed_buffer.h"
#include "mutex.h"
#include "outputer.h"

namespace muzi
{
class AsyncOutpter : public Outputer
{
public:
    void Output(const Outputer::Buffer &buffer) override;

    void Flush() override;

private:
    void Run();

    typedef FixedBuffer<config::kLargeBuffSize> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;

    const int flush_interval_;
    const std::string base_name_;
    const off_t roll_size_;
    std::atomic<bool> running_;

    MutexLock mutex_;
    Condition cond_;
    CountdownLatch latch_;

    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector input_buffers_;
    BufferVector output_buffers_;
};
}   // namespace muzi

#endif  // MUZI_BASE_ASYN_LOG_H_
