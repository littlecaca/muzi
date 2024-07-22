#include "logger.h"

#include "errno.h"

#include "current_thread.h"
#include "string_proxy.h"

namespace muzi
{

namespace 
{
thread_local char t_errno_buf[512];

const char *GetError(error_t errcode)
{
    return strerror_r(errcode, t_errno_buf, sizeof t_errno_buf);
}

}   // internal linkage

Logger gDefaultLogger;
Logger gStdioLogger;
Logger gStderrLogger(&gStderrOutputer);

const char * const kLogLevelName[LogLevel::kLogLevelNum] = 
{
    "Trace",
    "Debug",
    "Info ",
    "Warn ",
    "Error",
    "Fatal",
};

StackWritter::StackWritter(const Logger &logger, const SourceFile &file, int line,
    LogLevel level, error_t errcode) : logger_(logger), level_(level), file_(file), line_(line)
{
    current_thread::CachedTid();
    log_stream_ << "[";
    log_stream_ << TimeStamp().ToFormatString() << " ";
    log_stream_ << current_thread::t_tid_string << " ";
    log_stream_ << StringProxy(kLogLevelName[level_], 5);

    if (errcode != 0)
    {
        log_stream_ << GetError(errcode) << " (errno = " << errcode << ")";
    }
    log_stream_ << "] ";
}

void StackWritter::Finish()
{
    log_stream_ << " " << StringProxy(file_.data_, file_.size_) << " : " << line_ << "\n";
}


}   // namespace muzi
