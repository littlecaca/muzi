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

    virtual void Output(const Buffer &buf)
    {
        std::cout << buf.ToCStr() << "\n";
    }

    virtual void Flush()
    {
        std::cout << std::flush;
    }
};

extern Outputer *gDefaultOutputer;

}   // namespace muzi


#endif  // MUZI_BASE_OUTPUTER_H_