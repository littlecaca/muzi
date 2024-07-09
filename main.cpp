#include "mutex.h"
#include "debug.h"

class Foo : muzi::noncopyable
{

};

int main(int argc, char const *argv[])
{
    PRINT("main");
    muzi::MutexLock lock1, lock2;
    return 0;
}
