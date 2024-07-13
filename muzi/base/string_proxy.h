#ifndef MUZI_BASE_STRING_PROXY_H_
#define MUZI_BASE_STRING_PROXY_H_

#include <string.h>
#include <string>

namespace muzi
{
// A simple enclosion for plain c style string
// It does not affect the life cycle of the string
// It is suitable to manage on-stack object
// It acts like a std::string
class StringProxy
{
public:
    StringProxy() : ptr_(NULL), length_(0) {}
    StringProxy(const char *ptr) 
        : ptr_(ptr), length_(strlen(ptr)) {}
    StringProxy(const char *ptr, int len) 
        : ptr_(ptr), length_(len) {}
    StringProxy(const std::string &str) 
        : ptr_(str.data()), length_(static_cast<size_t>(str.size())) {}

    const char *data() const
    {
        return ptr_;
    }

    bool empty() const 
    {
        return length_ == 0;
    }

    size_t size() const { return length_; }

    const char *begin() const
    {
        return ptr_;
    }

    const char *end() const
    {
        return ptr_ + length_;
    }

    void reset()
    {
        ptr_ = NULL;
        length_ = 0;
    }

    void reset(const char *ptr)
    {
        ptr_ = ptr;
        length_ = strlen(ptr);
    }

    void reset(const char *ptr, int len)
    {
        ptr_ = ptr;
        length_ = len;
    }

    void reset(const std::string &str)
    {
        ptr_ = str.data();
        length_ = str.size();
    }

    char operator[](size_t index) const
    {
        return ptr_[index];
    }

    bool operator==(const StringProxy &rhs)
    {
        return (length_ == rhs.length_ && memcmp(ptr_, rhs.ptr_, length_) == 0);
    }

    bool operator!=(const StringProxy &rhs)
    {
        return !(*this == rhs);
    }

    int compare(const StringProxy &rhs) const
    {
        int res = memcmp(ptr_, rhs.ptr_, length_ > rhs.length_ ? rhs.length_ : length_);
        if (res == 0)
        {
            if (length_ > rhs.length_) return 1;
            else if (length_ < rhs.length_) return -1;
        }
        return res;
    }

    #define COMPARE_OPERATOR(op, aux_op)                    \
        bool operator op (const StringProxy &rhs) const     \
        {                                                   \
            int res = memcmp(ptr_, rhs.ptr_, length_        \
                > rhs.length_ ? rhs.length_ : length_);     \
            return res aux_op 0 || (res == 0                \
                    && length_ op rhs.length_);             \
        }                               

    COMPARE_OPERATOR(<, <)
    COMPARE_OPERATOR(<=, <)
    COMPARE_OPERATOR(>, >)
    COMPARE_OPERATOR(>=, >)                    

    #undef COMPARE_OPERATOR
    


    std::string ToString() const
    {
        return std::string(ptr_, length_);
    }

    void CopyToString(std::string *str) const
    {
        str->assign(ptr_, length_);
    }


private:
    const char *ptr_;
    size_t length_;
};


}   // namespace muzi


#endif  // MUZI_BASE_STRING_PROXY_H_