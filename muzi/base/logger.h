#ifndef MUZI_BASE_LOGGER_H_
#define MUZI_BASE_LOGGER_H_

#include <errno.h>
#include <string.h>
#include <utility>

#include "log_stream.h"
#include "noncopyable.h"
#include "timestamp.h"
#include "outputer.h"

namespace muzi
{
enum LogLevel
{
    kTrace,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kFatal,
    kLogLevelNum,
};

// Get the source file's basename
class SourceFile
{
public:
    template <int N>
    SourceFile(const char (&file)[N]);

    const char *data_;
    int size_;
};

// RAII style
class StackWritter : noncopyable
{
    friend class Logger;
public:
    template <typename T>
    StackWritter &operator<<(T &&msg)
    {
        log_stream_ << std::forward<T>(msg);
        return *this;
    }

    StackWritter(Outputer *outputer, const SourceFile &file, int line) 
        : StackWritter(outputer, file, line, LogLevel::kInfo, 0) {}

    StackWritter(Outputer *outputer, const SourceFile &file, int line, LogLevel level)
        : StackWritter(outputer, file, line, level, 0) {}

    StackWritter(Outputer *outputer, const SourceFile &file, int line, LogLevel level, const char *func) 
        : StackWritter(outputer, file, line, level, 0)
    {
       log_stream_ << func << ' ';
    }

    StackWritter(Outputer *outputer, const SourceFile &file, int line, bool to_abort)
        : StackWritter(outputer, file, line, to_abort ? LogLevel::kFatal : LogLevel::kError, errno) {}

    ~StackWritter()
    {
        Finish();
        outputer_->Output(log_stream_.GetBuffer());
        if (level_ == LogLevel::kFatal)
        {
            outputer_->Flush();
            abort();
        }
    }

private:
    StackWritter(Outputer *outputer, const SourceFile &file, int line, LogLevel level, error_t errcode);

    void Finish();

private:
    LogStream log_stream_;
    Outputer *outputer_;
    LogLevel level_;
    SourceFile file_;
    int line_;
};

// Set StackWriter as the default Writter
class Logger
{
public:
    using Writter = StackWritter;

    // Expecting the copy ellision to provide performance
    template <typename ...T>
    Writter Log(T &&...args)
    {
        return Writter(outputer_, std::forward<T>(args)...);
    }
    
    LogLevel GetLogLevel() const { return log_level_; }
    Outputer *GetOutputer() const { return outputer_; }

    void SetLogLevel(LogLevel level)
    {
        log_level_ = level;
    }

    void SetOutputer(Outputer *outputer)
    {
        outputer_ = outputer;
    }

protected:
    LogLevel log_level_;
    Outputer *outputer_;
};


// Default and static logger for quick and easy to use log system
extern Logger &global_logger;

#define LOG_TRACE global_logger.Trace()
#define LOG_DEBUG global_logger.Debug()
#define LOG_INFO  global_logger.Trace()
#define LOG_WARN  global_logger.Warn()
#define LOG_ERROR global_logger.Error()
#define LOG_FARAL global_logger.Fatal()


// Template implementation

template <int N>
SourceFile::SourceFile(const char (&file)[N]) : data_(file), size_(N)
{
    const char *right = file[N - 2];
    while (right >= file)
    {
        if (*right-- == '/')
            break;
    }
    
    ++right;
    if (right != file)
    {
        data_ = right;
        size_ -= right - file;

    }
}


}   //  namespace muzi


#endif  // MUZI_BASE_LOGGER_H_
