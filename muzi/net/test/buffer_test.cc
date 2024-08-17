#include "buffer.h"
#include "logger.h"
#include "timestamp.h"
#include "current_thread.h"

using namespace muzi;

int main(int argc, char const *argv[])
{
    LOG_INFO << "Test start in " << muzi::current_thread::tid() 
             << " in " << muzi::Timestamp().ToFormatString();
    muzi::gDefaultOutputer.Flush();
    
    muzi::Buffer buf;
    
    LOG_INFO << "Initial WritableBytes(): " << buf.WritableBytes();
    muzi::gDefaultOutputer.Flush();
    assert(buf.WritableBytes() == Buffer::kBufferSize - Buffer::kCheapPrepend - 1);


    char message[] = "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!This is not a test!!!"
    "This is not a test!!!This is not a test!!!END";

    
    buf.Append(message, sizeof message);
    
    LOG_INFO << "After one Append 547 bytes WritableBytes(): " << buf.WritableBytes();
    muzi::gDefaultOutputer.Flush();

    assert(buf.WritableBytes() == muzi::Buffer::kBufferSize - 
        sizeof message - 1 - muzi::Buffer::kCheapPrepend);

    LOG_INFO << "After one Append 547 bytes ReadableBytes() : " << buf.ReadableBytes();
    assert(buf.ReadableBytes() == sizeof message);


    std::string output = buf.RetriveAllAsString();

    LOG_INFO << output;

    assert(buf.WritableBytes() == Buffer::kBufferSize - Buffer::kCheapPrepend - 1);
    assert(buf.ReadableBytes() == 0);

    for (int i = 0; i < 10; ++i)
    {
        buf.Append(message, sizeof message);
    }

    assert(buf.ReadableBytes() == 10 * sizeof message);

    LOG_INFO << "After 10 Append 547 bytes WritableBytes() : " << buf.WritableBytes()
             << " ReadableBytes() : " << buf.ReadableBytes();

    LOG_INFO <<  buf.RetriveAllAsString();

    assert(buf.ReadableBytes() == 0);
    
    buf.AppendInt<uint32_t>(12345);
    assert(buf.PeekInt<uint32_t>() == 12345);
    buf.ReadInt<uint32_t>();
    
    buf.Append("this is very good\r\n");

    auto end = buf.FindCRLF();
    assert(end != buf.end());

    LOG_INFO << buf.RetriveAsString(end);

    buf.RetriveCRLF();
    assert(buf.ReadableBytes() == 0);
    
    
    return 0;
}
