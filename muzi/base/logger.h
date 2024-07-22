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
    Logger(Outputer *outputer = &gDefaultOutputer, LogLevel log_level = LogLevel::kTrace) 
        : outputer_(outputer), log_level_(log_level) {}
    Logger(LogLevel log_level) : Logger(&gDefaultOutputer, log_level) {}

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
    Outputer *outputer_;
    LogLevel log_level_;
};

// Default and static logger for quick and easy to use log system
extern Logger gDefaultLogger;
extern Logger gStdioLogger;
extern Logger gStderrLogger;


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

    StackWritter(const Logger &logger, const SourceFile &file, int line, LogLevel level, const char *func) 
        : StackWritter(logger, file, line, level, 0)
    {
       log_stream_ << func << ": ";
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


#define LOG_TRACE_U(logger) if (logger.GetLogLevel() <= muzi::LogLevel::kTrace) \
    muzi::Writter(logger, __FILE__, __LINE__, muzi::LogLevel::kTrace, __func__).GetStream()
#define LOG_DEBUG_U(logger) if (logger.GetLogLevel() <= muzi::LogLevel::kDebug) \
    muzi::Writter(logger, __FILE__, __LINE__, muzi::LogLevel::kDebug, __func__).GetStream()
#define LOG_INFO_U(logger)  if (logger.GetLogLevel() <= muzi::LogLevel::kInfo) \
    muzi::Writter(logger, __FILE__, __LINE__).GetStream()
#define LOG_WARN_U(logger)  if (logger.GetLogLevel() <= muzi::LogLevel::kWarn) \
    muzi::Writter(logger, __FILE__, __LINE__, muzi::LogLevel::kWarn).GetStream()
#define LOG_ERROR_U(logger)  muzi::Writter(logger, __FILE__, __LINE__, muzi::LogLevel::kError).GetStream()
#define LOG_FATAL_U(logger)  muzi::Writter(logger, __FILE__, __LINE__, muzi::LogLevel::kFatal).GetStream()
#define LOG_SYSERR_U(logger) muzi::Writter(logger, __FILE__, __LINE__, false).GetStream()
#define LOG_SYSFAT_U(logger) muzi::Writter(logger, __FILE__, __LINE__, true).GetStream()

// Interface
#define LOG_TRACE   LOG_TRACE_U(muzi::gDefaultLogger)
#define LOG_DEBUG   LOG_DEBUG_U(muzi::gDefaultLogger)
#define LOG_INFO    LOG_INFO_U(muzi::gDefaultLogger)
#define LOG_WARN    LOG_WARN_U(muzi::gDefaultLogger)
#define LOG_ERROR   LOG_ERROR_U(muzi::gDefaultLogger)
#define LOG_FATAL   LOG_FATAL_U(muzi::gDefaultLogger)
#define LOG_SYSERR  LOG_SYSERR_U(muzi::gDefaultLogger)
#define LOG_SYSFAT  LOG_SYSFAT_U(muzi::gDefaultLogger)

// Template implementation

template <int N>
SourceFile::SourceFile(const char (&file)[N]) : data_(file), size_(N - 1)
{
    const char *right = &file[N - 2];
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
