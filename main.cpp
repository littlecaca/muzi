#include <fstream>
#include <iostream>
#include <stdio.h>

#include "mutex.h"



int main(int argc, char const *argv[])
{
    std::ios::sync_with_stdio(false);

    std::ofstream file("dsa");
    std::ifstream ifile("dsa");
    
    return 0;
}
