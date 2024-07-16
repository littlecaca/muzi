#include "timestamp.h"


#include <unistd.h>
#include <iostream>

#include "timezone.h"


int main(int argc, char const *argv[])
{
    std::cout << muzi::TimeStamp().ToFormatString() << std::endl;
    muzi::TimeStamp::SetTimeZone(&muzi::kLocalTimeZone);
    sleep(1);
    std::cout << muzi::TimeStamp().ToFormatString() << std::endl;
    
    std::cout << muzi::TimeStamp(12345).ToFormatString() << std::endl;
    

    return 0;
}
