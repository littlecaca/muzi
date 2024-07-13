#ifndef MUZI_BASE_CONFIG_H_
#define MUZI_BASE_CONFIG_H_


namespace muzi
{
namespace config
{
    // Used by LogStream
    constexpr int kSmallBuffSize = 4096;
    // Used by AsyncLogging
    constexpr int kLargeBuffSize = 4096 * 1000;
}
}

#endif  // MUZI_BASE_CONFIG_H_