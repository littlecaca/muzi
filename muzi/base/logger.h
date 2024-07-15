#ifndef MUZI_LOG_LOGGER_H_
#define MUZI_LOG_LOGGER_H_

#include <string.h>

#include "log_stream.h"
#include "noncopyable.h"
#include "timestamp.h"


namespace muzi
{
class Logger
{
public:
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

    
    virtual const char *GetLogFile() const = 0;
    virtual LogLevel GetLogLevel() const = 0;
    virtual void SetLogLevel() = 0;

protected:
    LogLevel log_level_;

    
};

// Default and static logger for quick and easy to use log system
extern const Logger &global_logger;

#define LOG_TRACE global_logger.Trace()
#define LOG_DEBUG global_logger.Debug()
#define LOG_INFO  global_logger.Trace()
#define LOG_WARN  global_logger.Warn()
#define LOG_ERROR global_logger.Error()
#define LOG_FARAL global_logger.Fatal()

// Get the source file's basename
class SourceFile
{
public:
    template <int N>
    SourceFile(const char (&file)[N]);


private:
    const char *data_;
    int size_;

};

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


// RAII style
class StackWriter : noncopyable
{
public:
    StackWriter(SourceFile file, int line, Logger::LogLevel level, error_t errno);

    StackWriter(SourceFile file, int line, Logger::LogLevel level, const char *func);

    StackWriter(SourceFile file, int line, Logger::LogLevel level);

    ~StackWriter();



private:
    LogStream log_stream_;
    TimeStamp time_;
};

}   //  namespace muzi


#endif  // MUZI_LOG_LOGGER_H_
