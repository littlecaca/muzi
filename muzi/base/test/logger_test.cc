#include "logger.h"
#include "timestamp.h"


void DoTest()
{
    LOG_TRACE << "Do the Test" << " good engough";
    LOG_ERROR << "Report an error";
}



int main(int argc, char const *argv[])
{
    muzi::Timestamp::SetTimeZone(&muzi::kLocalTimeZone);
    LOG_ERROR << "Error!";
    LOG_INFO << "Success!!!";
    LOG_TRACE << "trace msh";

    muzi::Logger mylogger(muzi::kError);

    LOG_INFO_U(mylogger) << "this is me";
    LOG_ERROR_U(mylogger) << "this is not me";

    DoTest();

    return 0;
}
