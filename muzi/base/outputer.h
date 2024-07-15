#ifndef MUZI_BASE_OUTPUTER_H_
#define MUZI_BASE_OUTPUTER_H_

#include <iostream>

#include "noncopyable.h"
#include "fixed_buffer.h"
#include "config.h"

namespace muzi
{
class Outputer : noncopyable
{
public:
    typedef FixedBuffer<config::kSmallBuffSize> Buffer;
    virtual void Output(Buffer &buf)
    {
        std::cout << buf.ToCStr() << std::endl;
    }
};

}   // namespace muzi


#endif  // MUZI_BASE_OUTPUTER_H_