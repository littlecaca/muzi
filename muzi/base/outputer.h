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
    typedef FixedBuffer<config::kSmallBuffSize> SmallBuffer;

    virtual void Output(const SmallBuffer &buf)
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
    void Output(const SmallBuffer &buf) override
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

class ClosedOuputer : public Outputer
{
public:
    void Output(const SmallBuffer &buf) override
    {
        // Do nothing
    }

    virtual void Flush()
    {
        // Do nothing
    }

    virtual ~ClosedOuputer()
    {
        // Do nothing
    }
};

extern Outputer gDefaultOutputer;
extern StderrOuputer gStderrOutputer;

}   // namespace muzi


#endif  // MUZI_BASE_OUTPUTER_H_
