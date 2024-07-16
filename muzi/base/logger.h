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

class Logger
{
public:
    Logger() : outputer_(gDefaultOutputer), log_level_(LogLevel::kTrace) {}

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

private:
    LogLevel log_level_;
    Outputer *outputer_;
};

// Default and static logger for quick and easy to use log system
extern Logger &gDefaultLogger;


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
public:
    template <typename T>
    StackWritter &operator<<(T &&msg)
    {
        log_stream_ << std::forward<T>(msg);
        return *this;
    }

    StackWritter(const Logger &logger, const SourceFile &file, int line) 
        : StackWritter(logger, file, line, LogLevel::kInfo, 0) {}

    StackWritter(const Logger &logger, const SourceFile &file, int line, LogLevel level)
        : StackWritter(logger, file, line, level, 0) {}

    StackWritter(const Logger &logger, const SourceFile &file, int line, LogLevel level, const char &func) 
        : StackWritter(logger, file, line, level, 0)
    {
       log_stream_ << func << ' ';
    }

    StackWritter(const Logger &logger, const SourceFile &file, int line, bool to_abort)
        : StackWritter(logger, file, line, to_abort ? LogLevel::kFatal : LogLevel::kError, errno) {}

    ~StackWritter()
    {
        Finish();
        logger_.GetOutputer()->Output(log_stream_.GetBuffer());
        if (level_ == LogLevel::kFatal)
        {
            logger_.GetOutputer()->Flush();
            abort();
        }
    }

    LogStream &GetStream() { return log_stream_; }

private:
    StackWritter(const Logger &logger, const SourceFile &file, int line, LogLevel level, error_t errcode);

private:
    LogStream log_stream_;
    void Finish();

private:
    const Logger &logger_;
    LogLevel level_;
    SourceFile file_;
    int line_;
};

// Set StackWritter as the default Writter
using Writter = StackWritter;

// Interface
#define LOG_TRACE_U(logger) if (logger.log_level <= LogLevel::kTrace) \
    Writter(logger, __file__, __line__, LogLevel::kTrace, __func__).GetStream()
#define LOG_DEBUG_U(logger) if (logger.log_level <= LogLevel::kDebug) \
    Writter(logger, __file__, __line__, LogLevel::kDebug, __func__).GetStream()
#define LOG_INFO_U(logger)  if (logger.log_level <= LogLevel::kInfo) \
    Writter(logger, __file__, __line__).GetStream()
#define LOG_WARN_U(logger)  if (logger.log_level <= LogLevel::kWarn) \
    Writter(logger, __file__, __line__, LogLevel::kWarn).GetStream()
#define LOG_ERROR_U(logger)  Writter(logger, __file__, __line__, LogLevel::kError).GetStream()
#define LOG_FATAL_U(logger)  Writter(logger, __file__, __line__, LogLevel::kFatal).GetStream()
#define LOG_SYSERR_U(logger) Writter(logger, __file__, __line__, false).GetStream()
#define Log_SYSFAT_U(logger) Writter(logger, __file__, __line__, true).GetStream()

#define LOG_TRACE   LOG_TRACE_U(gDefaultLogger)
#define LOG_DEBUG   LOG_DEBUG_U(gDefaultLogger)
#define LOG_INFO    LOG_INFO_U(gDefaultLogger)
#define LOG_WARN    LOG_WARN_U(gDefaultLogger)
#define LOG_ERROR   LOG_ERROR_U(gDefaultLogger)
#define LOG_FATAL   LOG_FATAL_U(gDefaultLogger)
#define LOG_SYSERR  LOG_SYSERR_U(gDefaultLogger)
#define Log_SYSFAT  Log_SYSFAT_U(gDefaultLogger)

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
