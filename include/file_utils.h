#ifndef MUZI_BASE_FILE_UTILS_H_
#define MUZI_BASE_FILE_UTILS_H_

#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <string>

#include "logger.h"

namespace muzi
{
namespace file_utils
{
class UnlockedWriteFile
{
public:
    UnlockedWriteFile() : fp(NULL) {}

    ~UnlockedWriteFile()
    {
        Close();
    }

    void Append(const char *msg, size_t len)
    {
        ::fwrite_unlocked(msg, 1, len, fp);
    }

    void Flush()
    {
        ::fflush_unlocked(fp);
    }

    void Reset(const std::string &file_name)
    {
        if (fp != NULL)
        {
            Close();
        }
        fp = fopen(file_name.data(), "w");
        if (!fp)
            LOG_SYSFAT_U(gStderrLogger) << "Can not open the file";
    }

    bool IsOpen() const 
    {
        return fp == NULL;
    }

    void Close()
    {
        if (::fclose(fp) == EOF)
        {
            LOG_ERROR_U(gStderrLogger) << "Can not close the file";
        }
    }
    
private:
    FILE *fp;
};

}
}   // namespace muzi


#endif  // MUZI_BASE_FILE_UTILS_H_
