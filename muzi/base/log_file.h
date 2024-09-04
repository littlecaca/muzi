#ifndef MUZI_BASE_LOG_FILE_H_
#define MUZI_BASE_LOG_FILE_H_

#include <string>

#include "noncopyable.h"
#include "string_proxy.h"
#include "file_utils.h"

namespace muzi
{

// not thread safe
class LogFile : noncopyable
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
    void Append(const StringProxy &str)
    {
        Append(str.data(), str.size());
    }
    // Unlocked flushing
    void Flush();
    void RollFile();

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

}   // namespace muzi
#endif  // MUZI_BASE_LOG_FILE_H_
