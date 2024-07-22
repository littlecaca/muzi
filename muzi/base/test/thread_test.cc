#include "thread.h"

#include <string>
#include <iostream>

#include <time.h>


void print(std::string msg)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000 * 1000 * 50;
    for (int i = 0; i < 100; ++i)
    {
        std::cout << msg << std::endl;
        nanosleep(&ts, NULL);
    }
}


int main(int argc, char const *argv[])
{
    muzi::Thread th(std::bind(print, "look"), "testing");
    th.Start();
    th.Join();
    return 0;
}
