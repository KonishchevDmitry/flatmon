#ifndef Util_Assertion_hpp
#define Util_Assertion_hpp

#include "Core.hpp"

#if UTIL_VERBOSE_ASSERTS
    #define UTIL_ASSERT(expression) do {       \
        if(!(expression))                      \
            ::Util::abort(__FILE__, __LINE__); \
    } while(false)
#else
    #define UTIL_ASSERT(expression) do { \
        if(!(expression))                \
            ::Util::abort();             \
    } while(false)
#endif

namespace Util {

#if UTIL_VERBOSE_ASSERTS
    void abort(const char* file, int line) __attribute__((__noreturn__));
#else
    void abort() __attribute__((__noreturn__));
#endif

}

#endif
