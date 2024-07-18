#include <fstream>
#include <iostream>

#include "mutex.h"



int main(int argc, char const *argv[])
{
    std::ios::sync_with_stdio(false);
    GUARDED_BY(1);

    return 0;
}
