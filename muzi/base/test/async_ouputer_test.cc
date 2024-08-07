#include "async_outputer.h"

#include <string>
#include <time.h>

#include "timestamp.h"
#include "logger.h"

int main(int argc, char const *argv[])
{
    // in scope
    muzi::AsyncOutputer outputer("TestLog");
    muzi::gDefaultLogger.SetOutputer(&outputer);
    outputer.Start();
    
    int cnt = 30;
    while (cnt--)
    {
        muzi::Timestamp start;
        LOG_INFO << "Hello logging system";
        for (int i = 0; i < 100; ++i)
        {
            LOG_INFO << std::string(3000, 'X');
            LOG_INFO << "HELLO my log system" << "GO!!";
        }

        muzi::Timestamp end;
        LOG_INFO_U(muzi::gStdioLogger) << (end - start).GetOriginalString();
    }

    return 0;
}
