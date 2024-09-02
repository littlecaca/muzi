#include "buffer.h"

#include <errno.h>
#include <sys/uio.h>


namespace muzi
{

const char Buffer::kCRLF[] = "\r\n";

ssize_t Buffer::ReadFd(int fd, int *saved_errno)
{
    DEBUGINFO << "Buffer::ReadFd()";

    // Create an on-stack extra buffer
    char extra_buf[kExtraBufferSize];

    // Create iovec and fill it with the buffer blocks
    BufferIter last = buffer_list_.end();
    int len = last - write_index_.GetBufferIter();
    struct iovec vec[len + 1];
    size_t writable = 0;

    vec[0].iov_base = write_index_.GetCur();
    vec[0].iov_len = write_index_.Writable();
    writable += vec[0].iov_len;

    int index = 1;
    for (BufferIter it = write_index_.GetBufferIter() + 1; it < last; ++it)
    {
        vec[index].iov_base = (*it)->begin();
        vec[index].iov_len = kBufferSize;
        ++index;
    }
    writable += (index - 1) * kBufferSize;

    // Keep the rail byte
    --vec[index - 1].iov_len;
    --writable;

    // assert the calculation is right
    assert(writable == WritableBytes());

    vec[len].iov_base = extra_buf;
    vec[len].iov_len = sizeof extra_buf;

    const ssize_t n = ::readv(fd, vec, len + 1);

    if (n < 0)
    {
        *saved_errno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        write_index_ += n;
    }
    else
    {
        write_index_ += writable;
        Append(extra_buf, n - writable);
    }
    return n;
}

ssize_t Buffer::WriteFd(int fd, int *saved_errno)
{
    DEBUGINFO << "Buffer::WriteFd()";
    // Create iovec and fill it with the buffer blocks
    BufferIter first = read_index_.GetBufferIter(),
               last =  write_index_.GetBufferIter();

    int len = last - first + 1;

    iovec vec[len];
    vec[0].iov_base = read_index_.GetCur();
    if (first == last)
    {
        vec[0].iov_len = write_index_ - read_index_;
    }
    else
    {
        vec[0].iov_len = read_index_.Writable();

        int index = 1;
        for (BufferIter it = first + 1; it < last; ++it)
        {
            vec[index].iov_base = (*it)->begin();
            vec[index].iov_len = kBufferSize;
            ++index;
        }
        vec[index].iov_base = (*last)->begin();
        vec[index].iov_len = write_index_.GetCurOffset();
    }

    ssize_t n = ::writev(fd, vec, len);

    if (n < 0)
    {
        *saved_errno = errno;
    }
    else
    {
        read_index_ += n;
    }
    return n;
}

/// @brief  Extend the Buffer.
void Buffer::ExtendSpace(size_t len)
{
    BufferIter left = read_index_.GetBufferIter();
    BufferIter right = write_index_.GetBufferIter();

    size_t read_index = left - buffer_list_.begin();
    size_t read_offset = static_cast<size_t>(read_index_.GetCurOffset());
    size_t offset = right - left;
    size_t write_offset = static_cast<size_t>(write_index_.GetCurOffset());

    int buffers_to_add = (len - 1) / kBufferSize + 1;
    int buffers_to_use = (buffer_list_.end() - left) + buffers_to_add;

    if (buffer_list_.size() > 2 * buffers_to_use)
    {
        // // Remove Me
        // LOG_DEBUG << "ExtendSpace(" << len << ") swap" << buffer_list_.size() * 2;
        // gDefaultOutputer.Flush();

        // Move data to the front.
        BufferIter idle_it = read_offset >= kCheapPrepend 
            ? buffer_list_.begin() : buffer_list_.begin() + 1;
        
        while (left <= right)
        {
            std::swap(*left++, *idle_it++);
        }

        read_index = read_offset >= kCheapPrepend
            ? 0 : 1;
    }
    else
    {
        // // Remove Me
        // LOG_DEBUG << "ExtendSpace(" << len << ") resize " << buffer_list_.size() * 2;
        // gDefaultOutputer.Flush();

        buffer_list_.resize(buffer_list_.size() * 2);
        AllocateBuffer(buffer_list_.begin() + buffer_list_.size() / 2, buffer_list_.end());
    }

    read_index_ = buffer_list_.begin() + read_index;
    read_index_ += read_offset;
    write_index_ = buffer_list_.begin() + read_index + offset;
    write_index_ += write_offset;
}

}   // namespace muzi
