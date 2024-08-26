#include "stacktrace.h"

#include "logger.h"

void print()
{
    LOG_INFO << muzi::StackTrace();
}

class Base
{
public:
    void func()
    {
        print();
    }
};

int main(int argc, char const *argv[])
{
    Base b;
    b.func();

    return 0;
}
