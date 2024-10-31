
#include "log_file.h"

#include "process_info.h"

namespace muzi
{


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

void LogFile::SetBaseName(const std::string &base_name)
{
    base_name_ = config::kLogPath;

    if (!base_name.empty() && base_name_.back() != '/')
        base_name_.push_back('/');

    base_name_ += base_name;
}

void LogFile::Append(const char *msg, size_t len)
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

void LogFile::Flush()
{
    file_.Flush();
}

void LogFile::RollFile()
{
    LOG_DEBUG_U(gStdioLogger) << "RollFile()";
    time_t now = ::time(NULL);
    std::string file_name = GetLogFileName(base_name_, now);
    Flush();
    last_roll_ = now;
    last_flush_ = now;
    
    file_.Reset(file_name);
}  

}   // namespace muzi
