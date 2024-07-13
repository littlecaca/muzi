#include <assert.h>
#include <iostream>

#include "fixed_buffer.h"

using namespace muzi;

int main(int argc, char const *argv[])
{
    FixedBuffer<4096> buffer;

    assert(buffer.empty());

    char msg[] = "test...test...";
    int len = sizeof msg;


    for (int i = 0; i < 3000; ++i)
        buffer.Append(msg, len - 1);

    assert(buffer.size() == 4095);
    assert(!buffer.empty());
    assert(buffer.GetAvail() == 0);

    std::cout << buffer.DebugStr() << std::endl;

    
    return 0;
}
