#include "muzi/base/noncopyable.h"
#include "muzi/base/mutex.h"

class Foo : muzi::noncopyable
{

};

int main(int argc, char const *argv[])
{
    muzi::MutexLock lock1, lock2;
    return 0;
}
