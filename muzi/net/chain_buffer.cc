#include "chain_buffer.h"
#include "logger.h"
#include <bits/types/struct_iovec.h>
#include <cerrno>
#include <sys/types.h>
#include <sys/uio.h>

namespace muzi
{
#define ExtraSpace 65536
#define MaxVecSize 16

ssize_t ChainBuffer::ReadFd(int fd, int *saved_errno)
{
    char extra_buf[ExtraSpace];
    struct iovec vec[MaxVecSize];
    BufferList::iterator peek_buffer = write_buffer_;
    Buffer::iterator peek_pos = write_pos_;
    int i = 0;
    ssize_t writable = 0;
    for (; peek_buffer != buffers_.end() && i < MaxVecSize - 1; 
        ++peek_buffer, peek_pos = peek_buffer->begin())
    {
        vec[i].iov_base = peek_pos;
        writable += (vec[i].iov_len = peek_buffer->end() - peek_pos);
        ++i;
    }
    
    vec[i].iov_base = extra_buf;
    vec[i].iov_len = sizeof extra_buf;
    ++i;

    const ssize_t readn = ::readv(fd, vec, i);
    LOG_DEBUG << "readv() return " << readn << " bytes";
    if (readn < 0)
    {
        *saved_errno = errno;
    }
    else 
    {
        if (static_cast<size_t>(readn) >= writable)
        {
            if (peek_buffer == buffers_.end())
            {
                buffers_.emplace_back();
                write_buffer_ = --buffers_.end();
                write_pos_ = write_buffer_->begin();
            }
            else
            {
                write_buffer_ = peek_buffer;
                write_pos_ = peek_pos;
            }
            readable_bytes_ += writable;
            Append(extra_buf, readn - writable);
            // LOG_DEBUG << StringProxy(extra_buf, readn - writable);
        }
        else
        {
            HasWrite(readn);
        }
    }
    return readn;
}

ssize_t ChainBuffer::WriteFd(int fd, int *saved_errno)
{
    struct iovec vec[MaxVecSize];
    
    int i = 0;
    BufferList::iterator peek_buffer = read_buffer_;
    Buffer::iterator peek_pos = read_pos_;

    while (peek_buffer != write_buffer_ && i < MaxVecSize)
    {
        vec[i].iov_base = peek_pos;
        vec[i].iov_len = (peek_buffer->end() - peek_pos);
        ++peek_buffer;
        peek_pos = peek_buffer->begin();
        ++i;
    }

    if (peek_buffer == write_buffer_ && i < MaxVecSize)
    {
        vec[i].iov_base = peek_pos;
        vec[i].iov_len = (write_pos_ - peek_pos);
        ++i;
    }

    const ssize_t writen = ::writev(fd, vec, i);

    if (writen < 0)
    {
        *saved_errno = errno;
    }
    else
    {
        Retrive(writen);
    }

    return writen;
}

}   // namespace muzi
