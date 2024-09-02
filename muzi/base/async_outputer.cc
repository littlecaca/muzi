#include "async_outputer.h"

#include <sys/stat.h>
#include <iostream>

#include "config.h"
#include "process_info.h"
#include "timestamp.h"

namespace muzi
{
AsyncOutputer *gAsyncOutputer = NULL;

AsyncOutputer::AsyncOutputer(const std::string &base_name)
    : log_file_(base_name), flush_interval_(3), running_(false), cond_(mutex_),
      latch_(1), thread_(std::bind(&AsyncOutputer::Run, this), "logging"), 
      current_buffer_(new LargeBuffer), next_buffer_(new LargeBuffer)
{
}

void AsyncOutputer::Start()
{   
    running_ = true;
    thread_.Start();
    latch_.Wait();
}

void AsyncOutputer::Stop()
{
    running_ = false;
    cond_.Notify();
    thread_.Join();
}

void AsyncOutputer::Flush()
{
    MutexLockGuard guard(mutex_);
    buffers_.push_back(std::move(current_buffer_));
    if (next_buffer_)
        current_buffer_ = std::move(next_buffer_);
    else
        current_buffer_.reset(new LargeBuffer);
    cond_.Notify();
}

void AsyncOutputer::Output(const Outputer::Buffer &buffer)
{
    MutexLockGuard guard(mutex_);

    if (buffer.size() > current_buffer_->GetAvail())
    {
        buffers_.push_back(std::move(current_buffer_));
        if (next_buffer_)
            current_buffer_ = std::move(next_buffer_);
        else
            current_buffer_.reset(new LargeBuffer);
        current_buffer_->Append(buffer.data(), buffer.size());
        cond_.Notify();
    }
    else
        current_buffer_->Append(buffer.data(), buffer.size());
}

void AsyncOutputer::Run()
{
    assert(running_ == true);
    
    BufferPtr new_buffer1(new LargeBuffer);
    BufferPtr new_buffer2(new LargeBuffer);
    BufferVector buffers_to_write;
    buffers_to_write.reserve(16);

    latch_.CountDown();
    bool last_write = true;
    
    while (running_ || last_write)
    {
        last_write = running_;
        
        // The extended double buffering technique
        {   
            // scoped
            MutexLockGuard guard(mutex_);
            if (buffers_.empty())
                cond_.WaitForSeconds(flush_interval_);
            buffers_.push_back(std::move(current_buffer_));
            current_buffer_ = std::move(new_buffer1);
            buffers_.swap(buffers_to_write);
            if (!next_buffer_)
                next_buffer_ = std::move(new_buffer2);
        }

        // Drop if there is too much data to write
        if (buffers_to_write.size() > 25)
        {
            std::cerr << "Dropped log message at " << Timestamp().ToFormatString() << ", "
                << buffers_to_write.size() - 2 << " large buffers" << std::endl;
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        // Write
        for (const auto &buffer : buffers_to_write)
        {
            log_file_.Append(buffer->data(), buffer->size());
        }

        if (buffers_to_write.size() > 2)
            buffers_to_write.resize(2);
        
        new_buffer1 = std::move(buffers_to_write.back());
        buffers_to_write.pop_back();
        new_buffer1->clear();

        if (!new_buffer2)
        {
            new_buffer2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer2->clear();
        }

        buffers_to_write.clear();
        log_file_.Flush();
    }
    log_file_.Flush();
}

namespace
{
std::string GetLogFileName(const std::string &base_name, time_t now)
{
    std::string file_name;
    file_name.reserve(base_name.size() + 64);
    file_name += base_name;

    struct tm time_buf;
    localtime_r(&now, &time_buf);
    char buf[32];
    strftime(buf, 32, ".%Y%m%d-%H%M%S.", &time_buf);
    file_name += buf;
    file_name += ProcessInfo::GetHostName();
    file_name += ".log";
    return file_name;
}

}   // internal linkage

void AsyncOutputer::LogFile::SetBaseName(const std::string &base_name)
{
    base_name_ = config::kLogPath;

    if (!base_name.empty() && base_name_.back() != '/')
        base_name_.push_back('/');

    base_name_ += base_name;
}

void AsyncOutputer::LogFile::Append(const char *msg, size_t len)
{
    file_.Append(msg, len);
    written_bytes_ += len;
    if (written_bytes_ >= kRollSize)
    {
        written_bytes_ = 0;
        RollFile();
    }
    else
    {
        ++count_;
        if (count_ >= kCheckEveryN)
        {
            count_ = 0;
            time_t now = ::time(NULL);
            if ((now - last_roll_) >= kRollIntervalSeconds)
            {
                RollFile();
            }
            else if (now - last_flush_ >= kFlushIntervalSeconds)
            {
                last_flush_ = now;
                Flush();
            }
        }
    }
}

void AsyncOutputer::LogFile::Flush()
{
    file_.Flush();
}

void AsyncOutputer::LogFile::RollFile()
{
    LOG_DEBUG_U(gStdioLogger) << "RollFile()";
    time_t now = ::time(NULL);
    std::string file_name = GetLogFileName(base_name_, now);
    Flush();
    last_roll_ = now;
    last_flush_ = now;
    
    file_.Reset(file_name);
}

} // namespace muzi
