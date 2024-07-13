#ifndef MUZI_BASE_LOG_STREAM_H_
#define MUZI_BASE_LOG_STREAM_H_

#include <stdint.h>
#include <string>
#include <type_traits>

#include "config.h"
#include "fixed_buffer.h"
#include "noncopyable.h"


namespace muzi
{
class LogStream : noncopyable
{
public:
    typedef FixedBuffer<config::kSmallBuffSize> Buffer;
    
    // Utilizing the SFINAE feature of C++ template
    // For integral type val
    template <typename T, typename std::enable_if<std::is_integral<T>::value, int> = 0>
    LogStream &operator<<(T val)
    {
        if (buffer_.GetAvail() >= kMaxNumericSize)
        {
            ConvertIneger(buffer_.end(), val);
        }
        return *this;
    }

    // For float type val
    template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int> = 0>
    LogStream &operator<<(T val)
    {
        if (buffer_.GetAvail() >= kMaxNumericSize)
        {
            ConvertFloat(buffer_.end(), val);
        }
        return *this;
    }
    
    // For other type using function overloading
    LogStream &operator<<(const char *msg)
    {
        if (msg)
        {
            buffer_.Append(msg, strlen(msg));
        }
        else
        {
            buffer_.Append("(null)", 6);
        }
        return *this;
    }
    
    LogStream &operator<<(const std::string &msg)
    {
        buffer_.Append(msg.c_str(), msg.size());
        return *this;
    }

    LogStream &operator<<(const StringProxy &str)
    {
        buffer_.Append(str.data(), str.size());
        return *this;
    }   

    LogStream &operator<<(const Buffer &buf)
    {
        *this << buf.ToStringProxy();
        return *this;
    }

    // Accept a restirct void * binary val
    LogStream &operator<<(const void *hex)
    {
        if (buffer_.GetAvail() >= kMaxHexSize)
        {
            ConvertHex(buffer_.end(), *reinterpret_cast<const uintptr_t *>(hex));
        }
    }

private:
    // Write integer type
    template <typename T>
    void ConvertIneger(char *buf, T val);

    // Write float type
    template <typename T>
    void ConvertFloat(char *buf, T val);

    // Write void * type as hex val
    void ConvertHex(char *buf, uintptr_t val);

private:
    Buffer buffer_;

    const int kMaxNumericSize = 48;
    const int kMaxHexSize = 8;
};

}   // namespace muzi

#endif  // MUZI_BASE_LOG_STREAM_H_