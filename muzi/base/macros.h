#ifndef MUZI_CONFIG_H_
#define MUZI_CONFIG_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// For compiler to expect the branch
#if defined(__GNUC__) || defined(__clang__)
    #define LIKELY(x)   __builtin_expect((x), 1)
    #define UNLIKELY(x) __builtin_expect((x), 0)
#else
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

// Assert the system function is 0
#define ISZERO(ret) ({ decltype(ret) errnum = (ret); \
                       if (UNLIKELY(errnum == 0)) assert(errnum == 0);   \
                       (errnum);})


#endif
