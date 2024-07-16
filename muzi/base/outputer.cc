#include "outputer.h"

namespace muzi
{
namespace
{
Outputer DefaultOutputer;
}   // internal linkage
Outputer *gDefaultOutputer = &DefaultOutputer;
}   // namespace muzi
