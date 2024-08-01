#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>


#include "mutex.h"

class Base
{
public:
    static int i;
};

class Derived : public Base
{
public:
    static int i;
};


int main(int argc, char const *argv[])
{
    std::ios::sync_with_stdio(false);

    std::ofstream file("dsa");
    std::ifstream ifile("dsa");

    std::vector<int> vi;
    vi.rbegin().base();
    auto bu = file.rdbuf();
    return 0;
}
