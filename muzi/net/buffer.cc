#include "buffer.h"

#include <errno.h>
#include <sys/uio.h>

const char muzi::Buffer::kCRLF[] = "\r\n";

ssize_t muzi::Buffer::ReadFd(int fd, int *saved_errno)
{
    char extra_buf[kExtraBufferSize];
    struct iovec vec[2];
    size_t writable = write_index_.Writable();
    vec[0].iov_base = write_index_.GetCur();
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof extra_buf;

    const ssize_t n = ::readv(fd, vec, 2);
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

/// @brief  Extend the Buffer.
void muzi::Buffer::ExtendSpace(size_t len)
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
        // Remove Me
        LOG_DEBUG << "ExtendSpace(" << len << ") swap" << buffer_list_.size() * 2;
        gDefaultOutputer.Flush();

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
        // Remove Me
        LOG_DEBUG << "ExtendSpace(" << len << ") resize " << buffer_list_.size() * 2;
        gDefaultOutputer.Flush();

        buffer_list_.resize(buffer_list_.size() * 2);
        AllocateBuffer(buffer_list_.begin() + buffer_list_.size() / 2, buffer_list_.end());
    }

    read_index_ = buffer_list_.begin() + read_index;
    read_index_ += read_offset;
    write_index_ = buffer_list_.begin() + read_index + offset;
    write_index_ += write_offset;
}
