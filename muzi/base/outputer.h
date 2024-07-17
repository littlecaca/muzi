#ifndef MUZI_BASE_OUTPUTER_H_
#define MUZI_BASE_OUTPUTER_H_

#include <iostream>
#include <stdio.h>

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
        fwrite(buf.data(), 1, buf.size(), stdout);
    }

    virtual void Flush()
    {
        fflush(stdout);
    }
};

extern Outputer gDefaultOutputer;

}   // namespace muzi


#endif  // MUZI_BASE_OUTPUTER_H_
