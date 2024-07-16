#ifndef MUZI_BASE_CONFIG_H_
#define MUZI_BASE_CONFIG_H_


namespace muzi
{
struct config
{
    // Used by LogStream
    static const int kSmallBuffSize = 4096;
    // Used by AsyncLogging
    static const int kLargeBuffSize = 4096 * 1000;
    // Logfile path
    static const char *kLogPath;
    // Roll size
    static const int kRollSize = 32;
};   // namespace config

}    // namespace muzi


#endif  // MUZI_BASE_CONFIG_H_
