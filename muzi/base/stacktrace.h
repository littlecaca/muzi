// Print Stack Backtrace Programmatically with Demangled Function Names
// Refer to
// https://panthema.net/2008/0901-stacktrace-demangled/

#ifndef MUZI_BASE_STACKTRACE_H_
#define MUZI_BASE_STACKTRACE_H_

#include <cxxabi.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

namespace muzi
{
/// @brief Get a demangled stack backtrace of the caller function.
/// @param max_frames Max layers to backtrace.
std::string StackTrace(unsigned int max_frames = 63);

}	// namespace muzi

#endif // _STACKTRACE_H_
