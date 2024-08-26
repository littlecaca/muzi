#include "traced_exception.h"

#include <string>

#include "stacktrace.h"

namespace muzi
{
Exception::Exception(StringProxy what)
    : message_(what.data())
{
    stack_ = StackTrace();
}

}   // namespace muzi
