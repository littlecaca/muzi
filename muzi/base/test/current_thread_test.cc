#include <iostream>

#include "current_thread.h"

using namespace muzi::current_thread;

int main(int argc, char const *argv[])
{
    std::cout << "tid: " << muzi::current_thread::tid() << std::endl;
    std::cout << "tid_string: " << muzi::current_thread::tid_string() << std::endl;
    std::cout << "tid_length: " << muzi::current_thread::tid_length() << std::endl;
    std::cout << "thread_name: " << muzi::current_thread::thread_name() << std::endl;

    assert(IsMainThread());

    SleepUsec(1000);
    

    return 0;
}
