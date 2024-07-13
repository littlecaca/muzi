#ifndef MUZI_BASE_LOG_STREAM_H_
#define MUZI_BASE_LOG_STREAM_H_

#include <stdint.h>
#include <algorithm>
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

    void ResetBuffer()
    {
        buffer_.clear();
    }

    const Buffer &GetBuffer() const { return buffer_; }

    void Append(const char *msg, size_t len) 
    { 
        buffer_.Append(msg, len); 
    }

public:
    // Operator<< overloads

    // Utilizing the SFINAE feature of C++ template
    // For integral type val
    template <typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    LogStream &operator<<(T val)
    {
        if (buffer_.GetAvail() >= kMaxNumericSize)
        {
            buffer_.Add(ConvertIneger(buffer_.end(), val));
        }
        return *this;
    }

    // For float type val
    template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    LogStream &operator<<(T val)
    {
        if (buffer_.GetAvail() >= kMaxNumericSize)
        {
            buffer_.Add(ConvertFloat(buffer_.end(), static_cast<double>(val)));
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
            buffer_.Add(ConvertHex(buffer_.end(), *reinterpret_cast<const uintptr_t *>(hex)));
        }
        return *this;
    }

private:
    // Write integer type
    template <typename T>
    size_t ConvertIneger(char *buf, T val);

    // Write float type
    size_t ConvertFloat(char *buf, double val)
    {
        return static_cast<size_t>(snprintf(buf, kMaxNumericSize, "%.12g", val));
    }

    // Write void * type as hex val
    size_t ConvertHex(char *buf, uintptr_t val);

private:
    Buffer buffer_;

    static const int kMaxNumericSize = 48;
    static const int kMaxHexSize = 8;
    static const char *kDigits;
    static const char *kZero;
    static const char *kDigitsHex;
};

template <typename T>
size_t LogStream::ConvertIneger(char *buf, T val)
{
    char *cur = buf;
    T i = val;
    
    do
    {
        *cur++ = kZero[i % 10];
        i /= 10;
    } while (i != 0);

    if (val < 0)
        *cur++ = '-';
    std::reverse(buf, cur);

    return static_cast<size_t>(cur - buf);
}

}   // namespace muzi

#endif  // MUZI_BASE_LOG_STREAM_H_
