#include "log_stream.h"

namespace muzi
{
namespace 
{
const char kDigits[] = "9876543210123456789";
const char *kZero = kDigits + 9;
const char kDigitsHex[] = "0123456789ABCDEF";

}   // internal linkage

template <typename T>
void LogStream::ConvertIneger(char *buf, T val)
{
    
}

template <typename T>
void LogStream::ConvertFloat(char *buf, T val)
{
    
}

void LogStream::ConvertHex(char *buf, uintptr_t val)
{

}

} // namespace muzi