#include "process_info.h"

namespace muzi
{
std::string ProcessInfo::GetHostName()
{
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0)
    {
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    }
    else
        return "unknownhost";
}
}   // namespace muzi