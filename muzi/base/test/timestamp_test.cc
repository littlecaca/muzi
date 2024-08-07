#include "timestamp.h"

#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "timezone.h"
#include "logger.h"


int main(int argc, char const *argv[])
{
    muzi::TimeStamp sum(0);

    for (int k = 0; k < 200; ++k)
    {
        muzi::TimeStamp start;

        std::cout << muzi::TimeStamp().ToFormatString() << std::endl;
        muzi::TimeStamp::SetTimeZone(&muzi::kLocalTimeZone);
        std::cout << muzi::TimeStamp().ToFormatString() << std::endl;
        
        std::cout << muzi::TimeStamp(12345).ToFormatString() << std::endl;
        
        std::vector<muzi::TimeStamp> tsv;
        for (int i = 0; i < 1000; ++i)
        {
            tsv.emplace_back();
        }

        int base = 1000;
        for (auto &ts : tsv)
        {
            ts.AddTime(base);
            --base;
        }

        sort(tsv.begin(), tsv.end());

        muzi::TimeStamp end;
        sum += (end - start);
    }
    

    LOG_TRACE << sum.GetUsecs() / 200;
    return 0;
}
