#include <fcntl.h>
#include <unistd.h>

#include "mutex.h"
#include "debug.h"
#include "singleton.h"

class Foo : muzi::noncopyable
{

};

int main(int argc, char const *argv[])
{
    print("main Test start...");

    int &i = muzi::Singleton<int>::getInstance();
    // i = 12;
    // int &i2 = muzi::Singleton<int>::instance();
    // print(i);
    // print(i2);
    // std::cout << ({1;2;}) << std::endl;
    print("main Test end...");
    return 0;
}
