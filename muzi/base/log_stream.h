#ifndef MUZI_BASE_LOG_STREAM_H_
#define MUZI_BASE_LOG_STREAM_H_

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

    LogStream &operator<<(const Buffer &buf)
    {
        // In case the buf is buffer_
        *this << buf.ToString();
        return *this;
    }

private:
    // Write integer type
    template <typename T>
    void ConvertIneger(char *buf, T val);

    // Write float type
    template <typename T>
    void ConvertFloat(char *buf, T val);

private:
    Buffer buffer_;

    const int kMaxNumericSize = 48;
};
}   // namespace muzi




#endif  // MUZI_BASE_LOG_STREAM_H_