#include "async_outputer.h"

namespace muzi
{
void AsyncOutpter::Run()
{
    assert(running_ == true);
    
    latch_.CountDown();
    BufferPtr new_buffer1(new Buffer);
    BufferPtr new_buffer2(new Buffer);

    BufferVector buffers_to_write;
    buffers_to_write.reserve(16);

    while (running_)
    {
        {   
            // scoped
            MutexLockGuard guard(mutex_);
            if (buffers_.empty());
        }
    }



}


}   // namespace muzi
