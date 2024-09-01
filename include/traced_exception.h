#ifndef MUZI_BASE_EXCEPTION_H_
#define MUZI_BASE_EXCEPTION_H_

#include <exception>
#include <string>

#include "string_proxy.h"

namespace muzi
{
class Exception : public std::exception
{
public:
    Exception(StringProxy what);

    const char *what() const noexcept override
    {
        return message_.data();
    }

    const char *GetStackTrace() const noexcept
    {
        return stack_.data();
    }

private:
    std::string message_;
    std::string stack_;
};  

}   // namespace muzi

#endif  // MUZI_BASE_EXCEPTION_H_
