#ifndef MUZI_NET_LINK_BUFFER_H_
#define MUZI_NET_LINK_BUFFER_H_

#include "string_proxy.h"
#include <array>
#include <cassert>
#include <cstring>
#include <iterator>
#include <cstdlib>
#include <list>
#include <string>
#include <sys/types.h>

#include "noncopyable.h"

namespace muzi
{
class ChainBuffer : muzi::noncopyable
{
public:
    static constexpr size_t kInitialBufferNum = 1;
    static constexpr size_t kBufferSize = 2048;

public:
    ChainBuffer() : buffers_(1),
        readable_bytes_(0),
        write_buffer_(buffers_.begin()),
        read_buffer_(buffers_.begin()),
        write_pos_(write_buffer_->begin()),
        read_pos_(read_buffer_->begin())
    {
    }
    
    size_t SlowReadableBytes() const
    {
        return std::distance(read_buffer_, write_buffer_) * kBufferSize + 
            (write_pos_ - write_buffer_->begin()) - (read_pos_ - read_buffer_->begin());
    }

    size_t ReadableBytes() const
    {
        // return std::distance(read_buffer_, write_buffer_) * kBufferSize + 
        //     (write_pos_ - write_buffer_->begin()) - (read_pos_ - read_buffer_->begin());
        return readable_bytes_;
    }

    size_t WritableBytes() const
    {
        return std::distance(static_cast<BufferList::const_iterator>(write_buffer_), 
            buffers_.end()) * kBufferSize - (write_pos_ - write_buffer_->begin());
    }

    void Retrive(size_t len)
    {
        assert(ReadableBytes() >= len);
        readable_bytes_ -= len;

        while (len > 0)
        {
            int m = static_cast<int>(read_buffer_->end() - read_pos_);
            if (m > len)
            {
                read_pos_ += len;
                len = 0;
            }
            else
            {
                auto temp = read_buffer_++;
                buffers_.splice(buffers_.end(), buffers_, temp);

                read_pos_ = read_buffer_->begin();
                len -= m;
            }
        }
    }

    void RetriveAll()
    {
        read_buffer_ = buffers_.begin();
        write_buffer_ = buffers_.begin();
        read_pos_ = read_buffer_->begin();
        write_pos_ = write_buffer_->begin();
        readable_bytes_ = 0;
    }

    std::string PeekAsString(size_t len) const
    {
        assert(ReadableBytes() >= len);
        auto peek_buffer = read_buffer_;
        auto peek_pos = read_pos_;
        
        std::string buf(len, 0);
        char *data = buf.data();

        while (len > 0)
        {
            auto from = peek_pos;
            int m = static_cast<int>(peek_buffer->end() - peek_pos);
            int n = 0;
            if (m > len)
            {
                n = len;
                len = 0;
            }
            else 
            {
                len -= m;
                n = m;
                ++peek_buffer;
                peek_pos = peek_buffer->begin();
            }
            ::memcpy(data, from, n);
            data += n;
        }
        return buf;
    }

    std::string PeekAllAsString() const
    {
        return PeekAsString(ReadableBytes());
    }

    std::string RetriveAsString(size_t len)
    {
        std::string ret(PeekAsString(len));
        Retrive(len);
        return ret;
    }

    std::string RetriveAllAsString()
    {
        std::string ret(PeekAllAsString());
        RetriveAll();
        return ret;
    }

    void Append(const char *data, size_t len)
    {
        readable_bytes_ += len;

        while (len > 0)
        {
            auto from = write_pos_;
            int n = 0;

            int m = static_cast<int>(write_buffer_->end() - write_pos_);
            if (m > len)
            {
                n = len;
                write_pos_ += len;
                len = 0;
            }
            else 
            {
                n = m;
                len -= m;
                if (std::next(write_buffer_) == buffers_.end())
                    buffers_.emplace_back();

                ++write_buffer_;
                write_pos_ = write_buffer_->begin();
            }

            ::memcpy(from, data, n);
            data += n;
        }
    }

    void Append(StringProxy sp)
    {
        Append(sp.data(), sp.size());
    }

    void Append(const std::string &str)
    {
        Append(str.data(), str.size());
    }

    void Append(const char *str)
    {
        Append(str, strlen(str));
    }

    void Append(const ChainBuffer &buffer)
    {
        BufferList::iterator peek_buffer = buffer.read_buffer_;
        Buffer::iterator peek_pos = buffer.read_pos_;
        for (; peek_buffer != buffer.write_buffer_; 
            ++peek_buffer, peek_pos = peek_buffer->begin())
        {
            Append(peek_pos, peek_buffer->end() - peek_pos);
        }

        Append(peek_pos, buffer.write_pos_ - peek_pos);
    }

    void UnWrite(size_t len)
    {
        assert(ReadableBytes() >= len);
        BackWard(write_buffer_, write_pos_, len);
        readable_bytes_ -= len;
    }

    void HasWrite(size_t len)
    {
        assert(WritableBytes() >= len);
        Forward(write_buffer_, write_pos_, len);
        readable_bytes_ += len;
    }

    void Shrink(size_t reserve)
    {
        if (WritableBytes() <= reserve) return;

        auto peek_buffer = write_buffer_;
        auto peek_pos = write_pos_;
        Forward(peek_buffer, peek_pos, reserve);
        buffers_.erase(++peek_buffer, buffers_.end());
    }

    ssize_t ReadFd(int fd, int *saved_errno);
    ssize_t WriteFd(int fd, int *saved_errno);

private:
    typedef std::array<char, kBufferSize> Buffer;
    typedef std::list<Buffer> BufferList;

    void Forward(BufferList::iterator &buffer_it, Buffer::iterator &buffer_pos, size_t len)
    {
        while (len > 0)
        {
            int n = static_cast<int>(buffer_it->end() - buffer_pos);
            if (n > len)
            {
                buffer_pos += len;
                len = 0;
            }
            else
            {
                ++buffer_it;
                buffer_pos = buffer_it->begin();
                len -= n;
            }
        }
    }

    void BackWard(BufferList::iterator &buffer_it, Buffer::iterator &buffer_pos, size_t len)
    {
        while (len > 0)
        {
            int n = static_cast<int>(buffer_pos - buffer_it->begin());
            if (n > len)
            {
                buffer_pos -= len;
                len = 0;
            }
            else
            {
                --buffer_it;
                buffer_pos = buffer_it->end();
                len -= n;
            }
        }
    }

private:
    BufferList buffers_;
    size_t readable_bytes_;

    BufferList::iterator write_buffer_;
    BufferList::iterator read_buffer_;
    Buffer::iterator write_pos_;
    Buffer::iterator read_pos_;
};
}   // namespace muzi
#endif  // MUZI_NET_LINK_BUFFER_H_
