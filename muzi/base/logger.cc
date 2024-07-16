#include "logger.h"

// #include "current_thread.h"

namespace muzi
{
StackWritter::StackWritter(Outputer *outputer, const SourceFile &file, int line,
    LogLevel level, error_t errcode) : outputer_(outputer), level_(level)
{
    
}


}   // namespace muzi