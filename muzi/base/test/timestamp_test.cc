#include "timestamp.h"

#include <iostream>

#include "timezone.h"


int main(int argc, char const *argv[])
{
    std::cout << muzi::TimeStamp().ToFormatString() << std::endl;
    std::cout << muzi::TimeStamp(muzi::kUtcTimeZone).ToFormatString() << std::endl;
    return 0;
}
