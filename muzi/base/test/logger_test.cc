#include "logger.h"

int main(int argc, char const *argv[])
{
    LOG_INFO << "Success!!!";
    LOG_ERROR << "Error!";
    LOG_TRACE << "trace msh";

    muzi::Logger mylogger(muzi::kError);

    LOG_INFO_U(mylogger) << "this is me";
    LOG_ERROR_U(mylogger) << "this is not me";

    return 0;
}
