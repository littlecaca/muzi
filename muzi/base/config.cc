#include "config.h"

#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "logger.h"

namespace muzi
{
const char *config::kLogPath = "./log/";

config::config()
{
    if (opendir(kLogPath) == NULL)
    {
        if (mkdir(kLogPath, 0770) == -1)
        {
            LOG_SYSFAT_U(gStderrLogger) << "Wrong log path";
        }
    }
}

namespace
{
// Validate the config arguments
config __config;
}   // internal linkage

}   // namespace muzi
