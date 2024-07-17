#include <algorithm>

#include "log_stream.h"

namespace muzi
{

const char *LogStream::kDigits = "9876543210123456789";
const char *LogStream::kZero = kDigits + 9;
const char *LogStream::kDigitsHex = "0123456789ABCDEF";

// thread_local LogStream::Buffer t_LogStreamBuffer;

size_t LogStream::ConvertHex(char *buf, uintptr_t val)
{
    char *cur = buf;
    uintptr_t i = val;

    do
    {
        *cur++ = kDigitsHex[i % 16];
        i /= 16;
    } while (i != 0);

    *cur++ = 'x';
    *cur++ = '0';
    std::reverse(buf, cur);

    return static_cast<size_t>(cur - buf);
}

} // namespace muzi
