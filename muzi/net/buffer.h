#ifndef MUZI_NET_BUFFER_H_
#define MUZI_NET_BUFFER_H_

#include <algorithm>
#include <assert.h>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "endian_transform.h"
#include "fixed_buffer.h"
#include "string_proxy.h"
#include "logger.h"

namespace muzi
{
/// @brief The data buffer for everry connection.
/// It keeps a memory in heap, and when reading data
/// it will create a stack memory, and use ::readv()
/// to read from system buffer. According to the requirement,
/// the heap memory can increase. In which case, the buffer
/// do not need to take up a very large memory in the begining.
class Buffer
{
public:
    static constexpr size_t kCheapPrepend = 8;
    static constexpr size_t kInitialBufferNum = 1;
    static constexpr size_t kBufferSize = 2048;
    static constexpr size_t kExtraBufferSize = 1024 * 64;


    typedef FixedBuffer<kBufferSize> BufferBlock;
    typedef std::unique_ptr<BufferBlock> BufferBlockPtr;
    typedef std::vector<BufferBlockPtr> BufferList;
    typedef BufferList::iterator BufferIter;

    /// @brief This Iterator can take std::vector or std::list as the BufferList.
    template <typename T>
    class __BufferIterator
    {
    public:
        // iterator_traits
        typedef std::random_access_iterator_tag iterator_category;
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        // For the conversion from iterator to const_iterator.
        typedef __BufferIterator<std::remove_const_t<T>> iterator;
        typedef __BufferIterator self;

        const static difference_type signed_buffer_size_ = static_cast<difference_type>(kBufferSize);

        __BufferIterator(BufferIter buffer_it)
            : buffer_it_(buffer_it),
              first_((*buffer_it)->begin()),
              last_(first_ + kBufferSize),
              cur_(first_)
        {
        }

        __BufferIterator() = default;

        __BufferIterator(BufferIter end_it, int mark)
            : buffer_it_(end_it),
              first_(0),
              last_(0),
              cur_(0)
        {
        }

        /// @brief Conversion from iterator to const_iterator.
        __BufferIterator(const iterator &it)
            : buffer_it_(it.GetBufferIter()), first_(it.GetFirst()), 
              last_(it.GetLast()), cur_(it.GetCur())
        {
        }

        void swap(self &rhs)
        {
            std::swap(buffer_it_, rhs.buffer_it_);
            std::swap(cur_, rhs.cur_);
            std::swap(first_, rhs.first_);
            std::swap(last_, rhs.last_);
        }

        reference operator*() const { return *cur_; };
        pointer operator->() const { return &operator*(); }

        friend difference_type operator-(const self &lhs, const self &rhs)
        {
            return signed_buffer_size_ * (std::distance(rhs.buffer_it_, lhs.buffer_it_) - 1) 
                + (lhs.cur_ - lhs.first_) + (rhs.last_ - rhs.cur_);
        }

        /// @brief Prefix self-increase
        self &operator++() &
        {
            if (++cur_ == last_)
            {
                MoveBuffer(1);
                cur_ = first_;
            }
            return *this;
        }

        /// @brief Suffix self-increase
        self operator++(int) &
        {
            self temp = *this;
            ++(*this);
            return temp;
        }

        /// @brief Prefix self-decrease
        self &operator--() &
        {
            if (cur_ == first_)
            {
                MoveBuffer(-1);
                cur_ = last_;
            }
            return *this;
        }

        /// @brief Suffix self-decrease
        self operator--(int) &
        {
            self temp = *this;
            --(*this);
            return temp;
        }

        /// @brief Skip to n offset
        self &operator+=(difference_type n) &
        {
            // Caculate the offset from the first_
            difference_type offset = n + (cur_ - first_);
            if (offset >= 0 && offset < signed_buffer_size_)
            {   
                cur_ += n;
            }
            else
            {
                difference_type buffer_offset =  offset > 0 
                    ? offset / signed_buffer_size_ : -((-offset - 1) / signed_buffer_size_) - 1;
                MoveBuffer(buffer_offset);
                cur_ = first_ + (offset - buffer_offset * signed_buffer_size_);
            }
            return *this;
        }

        self operator+(difference_type n) const
        {
            self temp = *this;
            return temp += n;
        }

        self &operator-=(difference_type n) &
        {
            return *this += -n;
        }

        self operator-(difference_type n) const
        {
            self temp = *this;
            return temp -= n;
        }

        reference operator[](difference_type n) const
        {
            return *(*this + n);
        }

        bool operator==(const self &rhs) const { return cur_ == rhs.cur_; } 
        bool operator!=(const self &rhs) const { return cur_ != rhs.cur_; }
        bool operator<(const self &rhs) const
        {
            return (buffer_it_ == rhs.buffer_it_) ? cur_ < rhs.cur_ : buffer_it_ < rhs.buffer_it_;
        }
        bool operator<=(const self &rhs) const
        {
            return (buffer_it_ == rhs.buffer_it_) ? cur_ <= rhs.cur_ : buffer_it_ < rhs.buffer_it_;
        }

    public:
        // For Buffer to use
        pointer GetFirst() const { return first_; }
        pointer GetCur() const { return cur_; }
        pointer GetLast() const { return last_; }
        const BufferIter &GetBufferIter() const { return buffer_it_; }
        size_t GetCurOffset() const { return cur_ - first_; }
        size_t Writable() const { return  std::max(last_ - cur_ - 1, 0L); }

        bool IsInSameBuffer(const self &rhs) const
        {
            return buffer_it_ == rhs.buffer_it_;
        }

        void MoveToNextBuffer()
        {
            MoveBuffer(1);
            cur_ = first_;
        }

    private:
        void MoveBuffer(difference_type offset)
        {
            std::advance(buffer_it_, offset);
            first_ = (*buffer_it_)->begin();
            last_ = first_ + kBufferSize;
        }

    private:
        BufferIter buffer_it_;
        pointer first_;
        pointer last_;
        pointer cur_;
    };

    typedef __BufferIterator<char> iterator;
    typedef __BufferIterator<const char> const_iterator;

public:
    explicit Buffer(size_t cheap_prepend = kCheapPrepend,
                    size_t initial_buffer_num = kInitialBufferNum)
        : buffer_list_(initial_buffer_num)
    {
        // Remove Me
        LOG_DEBUG << "Buffer()";
        gDefaultOutputer.Flush();
        AllocateBuffer(buffer_list_.begin(), buffer_list_.end());
        read_index_ = write_index_ = buffer_list_.begin();
        read_index_ += kCheapPrepend;
        write_index_ += kCheapPrepend;
    }

    void swap(Buffer &rhs)
    {
        buffer_list_.swap(rhs.buffer_list_);
        write_index_.swap(rhs.write_index_);
        read_index_.swap(rhs.read_index_);
    }

    const_iterator Peek() const { return cbegin(); }

    size_t ReadableBytes() const
    {
        return static_cast<size_t>(write_index_ - read_index_);
    }

    size_t WritableBytes()
    {
        // Keep one position in the end to avoid iterator validating.
        return std::max(iterator(buffer_list_.end(), 0) - write_index_ - 1, 0L);
    }

    size_t GetPrependableBytes()
    {
        return static_cast<size_t>(read_index_ - buffer_list_.begin());
    }

    const_iterator FindCRLF() const
    {
        return std::search(cbegin(), cend(), kCRLF, kCRLF + 2);
    }

    const_iterator FindCRLF(const_iterator start) const
    {
        assert(start <= cend());
        return std::search(start, cend(), kCRLF, kCRLF + 2);
    }

    const_iterator FindEOL() const
    {
        return std::find(cbegin(), cend(), '\n');
    }

    const_iterator FindEOL(const_iterator start) const
    {
        assert(start <= cend());
        return std::find(start, cend(), '\n');
    }

    const_iterator Find(StringProxy str) const
    {
        return std::search(cbegin(), cend(), str.begin(), str.end());
    }

    void Retrive(size_t len)
    {
        assert(len <= ReadableBytes());
        if (len < ReadableBytes())
        {
            read_index_ += len;
        }
        else
        {
            ResetIndex();
        }
    }

    void RetriveAll() { ResetIndex(); }

    void RetriveCRLF() { Retrive(2); }
    
    void RetriveEOL() { Retrive(1); }

    void RetriveUtil(const_iterator util)
    {
        assert(util <= cend());
        assert(cbegin() <= util);
        Retrive(util - cbegin());
    }

    void RetriveInt64() { Retrive(sizeof(int64_t)); }
    void RetriveInt32() { Retrive(sizeof(int32_t)); }
    void RetriveInt16() { Retrive(sizeof(int16_t)); }
    void RetriveInt8() { Retrive(sizeof(int8_t)); }

    std::string RetriveAllAsString()
    {
        return RetriveAsString(cend());
    }

    std::string RetriveAsString(size_t len)
    {
        return RetriveAsString(cbegin() + len);
    }
    
    std::string RetriveAsString(const_iterator util)
    {
        assert(util <= cend());
        assert(cbegin() <= util);
        // Remove Me
        LOG_DEBUG << "RetriveAsString " << util - cbegin();
        gDefaultOutputer.Flush();

        std::string output(cbegin(), util);
        Retrive(util - cbegin());
        return output;
    }

    void Append(const StringProxy &str)
    {
        Append(str.begin(), str.size());
    }

    void Append(const char *first, const char *last)
    {
        Append(first, last - first);
    }

    void Append(const void *first, size_t len)
    {
        EnsureWritableBytes(len);
        std::copy_n(static_cast<const char *>(first), len, end());
        write_index_ += len;
    }

    void EnsureWritableBytes(size_t len)
    {
        LOG_DEBUG << "EnsureWritbleBytes(" << len << ") Writable: " << WritableBytes();
        // Remove Me
        while (len > WritableBytes())
        {
            gDefaultOutputer.Flush();
            ExtendSpace(len - WritableBytes());
        }
    }

    void UnWrite(size_t len)
    {
        assert(len <= ReadableBytes());
        write_index_ -= len;
    }

    template <typename T>
    void AppendInt(T val)
    {
        T be = endian::HostToNet(val);
        Append(&be, sizeof be);
    }

    template <typename T>
    T ReadInt()
    {
        T res = PeekInt<T>();
        Retrive(sizeof(T));
        return res;
    }
    
    template <typename T>
    T PeekInt() const
    {
        assert(sizeof(T) <= ReadableBytes());
        char buf[sizeof(T) / sizeof(char)];
        std::copy(cbegin(), cbegin() + sizeof(T) / sizeof(char), buf);
        return endian::NetToHost(*reinterpret_cast<T *>(buf));
    }

    void Prepend(const void *first, size_t len)
    {
        assert(len <= GetPrependableBytes());
        read_index_ -= len;
        std::copy(static_cast<const char *>(first), 
            static_cast<const char *>(first) + len, begin());
    }

    template <typename T>
    void PrependInt(T val)
    {
        T be = endian::HostToNet(val);
        Prepend(&be, sizeof be);
    }

    /// @brief Read from fd directly to buffer.
    /// @return result of read(2), @c errno is saved.
    ssize_t ReadFd(int fd, int *saved_errno);

    const_iterator cbegin() const { return read_index_; }
    const_iterator cend() const { return write_index_; }
    iterator begin() { return read_index_; }
    iterator end() { return write_index_; }
    iterator begin() const { return read_index_; }
    iterator end() const { return write_index_; }

private:
    void ResetIndex()
    {
        // Remove Me
        LOG_DEBUG << "ResetIndex()";
        gDefaultOutputer.Flush();

        read_index_ = write_index_ = buffer_list_.begin();
        read_index_ += kCheapPrepend;
        write_index_ += kCheapPrepend;
    }

    /// @brief  Extend the Buffer.
    void ExtendSpace(size_t len);

    void AllocateBuffer(BufferIter first, BufferIter last)
    {
        // Remove Me
        LOG_DEBUG << "AllocateBuffer()";
        gDefaultOutputer.Flush();

        while (first != last)
        {
            *first++ = std::make_unique<BufferBlock>();
        }
    }

private:
    BufferList buffer_list_;
    iterator read_index_;
    iterator write_index_;

    static const char kCRLF[];
};

}   // namespace muzi

#endif  // MUZI_NET_BUFFER_H_
