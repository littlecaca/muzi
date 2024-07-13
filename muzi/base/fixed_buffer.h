#ifndef MUZI_BASE_FIXED_BUFFER_H_
#define MUZI_BASE_FIXED_BUFFER_H_

#include <stddef.h>
#include <string.h>
#include <string>

#include "string_proxy.h"
#include "noncopyable.h"

namespace muzi
{
template <size_t SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : end_(buffer_), storage_end_(buffer_ + sizeof buffer_) {}

    size_t size() const
    {
        return static_cast<size_t>(end() - begin());
    }

    size_t GetAvail() const
    {
        // Here -1 is to keep the '\0' in the end which is needed to generate DebugStr
        return static_cast<size_t>(storage_end_ - end() - 1);
    }
    
    char *end() const { return end_; }

    const char *begin() const { return buffer_; }

    const char *data() const { return buffer_; }

    // Update the end_ pointer to the first idle byte
    void Add(size_t len)
    {
        end_ += len;
    }

    // Clear the buffer
    void clear()
    {
        end_ = buffer_;
    }

    void Append(const char *msg, size_t len)
    {
        len = std::min(GetAvail(), len);
        memcpy(end_, msg, len);
        Add(len);
    }

    void SetZero()
    {
        memset(buffer_, 0, sizeof buffer_);
    }

    bool empty() const { return end_ == buffer_; }

    // Maybe used by gdb
    const char *DebugStr()
    {
        *end_ = '\0';
        return buffer_;
    }

    // Maybe used by unit test
    std::string ToString() const { return std::string(buffer_, size()); }

    StringProxy ToStringProxy() const { return StringProxy(buffer_, size()); }
    
private:
    char buffer_[SIZE];
    char *end_;
    const char *storage_end_;
};

}   // namespace muzi

#endif  // MUZI_BASE_FIXED_BUFFER_H_
