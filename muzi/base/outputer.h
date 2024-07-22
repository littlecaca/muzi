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

    virtual ~Outputer()
    {
        fflush(stdout);
    }
};

class StderrOuputer : public Outputer
{
public:
    void Output(const Buffer &buf) override
    {
        fwrite(buf.data(), 1, buf.size(), stderr);
    }

    virtual void Flush()
    {
        fflush(stderr);
    }

    virtual ~StderrOuputer()
    {
        fflush(stderr);
    }
};

extern Outputer gDefaultOutputer;
extern StderrOuputer gStderrOutputer;

}   // namespace muzi


#endif  // MUZI_BASE_OUTPUTER_H_
