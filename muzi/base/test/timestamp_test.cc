#include "timestamp.h"

#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "timezone.h"
#include "logger.h"


int main(int argc, char const *argv[])
{
    muzi::Timestamp sum(0);

    for (int k = 0; k < 200; ++k)
    {
        muzi::Timestamp start;

        std::cout << muzi::Timestamp().ToFormatString() << std::endl;
        muzi::Timestamp::SetTimeZone(&muzi::kLocalTimeZone);
        std::cout << muzi::Timestamp().ToFormatString() << std::endl;
        
        std::cout << muzi::Timestamp(12345).ToFormatString() << std::endl;
        
        std::vector<muzi::Timestamp> tsv;
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

        muzi::Timestamp end;
        sum += (end - start);
    }
    

    LOG_TRACE << sum.GetUsecs() / 200;
    return 0;
}
