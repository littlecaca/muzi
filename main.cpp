#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctime>
#include <iomanip>

#include "mutex.h"
#include "debug.h"
#include "singleton.h"

class Foo : muzi::noncopyable
{

};

int main(int argc, char const *argv[])
{
    print("main Test start...");
    std::time_t t = std::time(nullptr);
    
    // Get the local time
    std::tm local_tm = *std::localtime(&t);
    
    // Get the UTC time
    std::tm utc_tm = *std::gmtime(&t);
    
    // Calculate the timezone offset in seconds
    std::time_t local_time = std::mktime(&local_tm);
    std::time_t utc_time = std::mktime(&utc_tm);
    int timezone_offset = static_cast<int>(std::difftime(local_time, utc_time));
    
    // Print the local time and timezone offset
    std::cout << "Local time: " << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << '\n';
    std::cout << "UTC time: " << std::put_time(&utc_tm, "%Y-%m-%d %H:%M:%S") << '\n';
    std::cout << "Timezone offset: " << (timezone_offset / 3600) << " hours" << '\n';

    print("main Test end...");
    return 0;
}
