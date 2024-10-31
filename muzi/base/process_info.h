#ifndef MUZI_BASE_PROCESS_INFO_H_
#define MUZI_BASE_PROCESS_INFO_H_

#include <string>

#include <unistd.h>

#include "noncopyable.h"

namespace muzi
{
class ProcessInfo : noncopyable
{
public:
    static std::string GetHostName();
};

}   // namespace muzi

#endif  // MUZI_BASE_PROCESS_INFO_H_
