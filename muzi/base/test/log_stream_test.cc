#include "log_stream.h"

#include <iostream>
#include <string>
#include <type_traits>

#include "fixed_buffer.h"
#include "string_proxy.h"


int main(int argc, char const *argv[])
{
    muzi::LogStream ls;
    char msg[] = "This is a message";
    char *p = msg;
    ls << p << "\n";
    ls << "int 32: " << 32 << "\n";
    ls << "int 211321: " << 211321 << "\n";
    ls << muzi::StringProxy("unsigned long long 32: ") << 32ULL << "\n";
    ls << muzi::StringProxy("long long -33242: ") << -33242LL << "\n";
    ls << "float -32.0: " << -32.0F << "\n";
    ls << std::string("double 32.21231: ") << 32.21231 << "\n";

    ls.Append(msg, sizeof msg);

    std::cout << ls.GetBuffer().data() << std::endl;
    
    return 0;
}
