#ifndef MUZI_LOG_LOGGER_H_
#define MUZI_LOG_LOGGER_H_

#include <errno.h>
#include <string.h>
#include <utility>

#include "log_stream.h"
#include "noncopyable.h"
#include "timestamp.h"
#include "outputer.h"


namespace muzi
{
enum class LogLevel
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


private:
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

    StackWritter(SourceFile file, int line) 
        : StackWritter(file, line, LogLevel::kInfo, 0) {}
    StackWritter(SourceFile file, int line, LogLevel level)
        : StackWritter(file, line, level, 0) {}
    StackWritter(SourceFile file, int line, LogLevel level, const char *func) 
        : StackWritter(file, line, level, 0)
    {
        log_stream_ << func << ' ';
    }
    StackWritter(const SourceFile &file, int line, LogLevel level, error_t errcode);
    StackWritter(const StringProxy &str);

    ~StackWritter();

private:
    TimeStamp time_;
    LogStream log_stream_;
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
        return Writter(std::forward<T>(args)...);
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


#endif  // MUZI_LOG_LOGGER_H_
