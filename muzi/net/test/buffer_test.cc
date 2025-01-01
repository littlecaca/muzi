#include "buffer.h"
#include "logger.h"
#include "outputer.h"
#include "timestamp.h"
#include "current_thread.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

using namespace muzi;

int main(int argc, char const *argv[])
{
    LOG_INFO << "Test start in " << muzi::current_thread::tid() 
             << " in " << muzi::Timestamp().ToFormatString();
    muzi::gDefaultOutputer.Flush();
    
    muzi::Buffer buf;
    
    LOG_INFO << "Initial WritableBytes(): " << buf.WritableBytes();
    muzi::gDefaultOutputer.Flush();
    // assert(buf.WritableBytes() == Buffer::kBufferSize - Buffer::kCheapPrepend - 1);


    char message[] = "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!END";

    // Write one message
    buf.Append(message, sizeof message - 1);

    assert(buf.ReadableBytes() == sizeof message - 1);
        
    LOG_INFO << "After one Append " << (sizeof message - 1) <<  
        " bytes WritableBytes(): " << buf.WritableBytes();

    muzi::gDefaultOutputer.Flush();

    LOG_INFO << buf.RetriveAllAsString();

    assert(buf.ReadableBytes() == 0);

    // write 10 messages
    for (int i = 0; i < 10; ++i)
    {
        buf.Append(message, sizeof message - 1);
    }
    std::cout << buf.PeekAllAsString() << std::endl;

    assert(buf.ReadableBytes() == 10 * (sizeof message - 1));

    LOG_INFO << "After 10-times Append " << (sizeof message - 1) <<  
        " bytes, WritableBytes(): " << buf.WritableBytes() <<
        " bytes, ReadableBytes(): " << buf.ReadableBytes() << " bytes";
    muzi::gDefaultOutputer.Flush();

    // LOG_INFO <<  buf.RetriveAllAsString();

    // assert(buf.ReadableBytes() == 0);
    
    // // buf.AppendInt<uint32_t>(12345);
    // // assert(buf.PeekInt<uint32_t>() == 12345);
    // // buf.ReadInt<uint32_t>();
    
    // buf.Append("this is very good\r\n");

    // // auto end = buf.FindCRLF();
    // // assert(end != buf.end());

    // // LOG_INFO << buf.RetriveAsString(end);

    // // buf.RetriveCRLF();
    // // assert(buf.ReadableBytes() == 0);
    
    // write to fd
    buf.RetriveAll();
    assert(buf.ReadableBytes() == 0);

    int saved_errno = 0;
    int n = buf.WriteFd(1, &saved_errno);
    if (saved_errno < 0)
    {
        LOG_SYSERR << "buf.WriteFd()";
    }
    assert(n == 0);

    for (int i = 0; i < 10; ++i)
    {
        buf.Append(message, sizeof message - 1);
    }

    n = buf.WriteFd(1, &saved_errno);
    if (saved_errno < 0)
    {
        LOG_SYSERR << "buf.WriteFd()";
    }
    assert(n != 0);
    assert(buf.ReadableBytes() == 0);

    LOG_INFO << "After 10 Append message, WriteFd() : " << n;

    buf.RetriveAll();

    // read fd
    int fd = open("text_input.txt", O_RDONLY);
    int ret;
    while ((ret = buf.ReadFd(fd, &saved_errno)) > 0)
    {
        std::cout << buf.RetriveAllAsString() << std::endl;
    }
    LOG_INFO << "ret value: " << ret;
    
    if (ret < 0)
    {
        LOG_ERROR << strerror(saved_errno);
    }

    assert(ret >= 0);
    

    // write buffer
    buf.RetriveAll();
    

    LOG_INFO << "done";
    

    return 0;
}
