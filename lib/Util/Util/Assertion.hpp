#ifndef Util_Assertion_hpp
#define Util_Assertion_hpp

#include "Core.hpp"

#if UTIL_VERBOSE_ASSERTS
    #define UTIL_ASSERT(expression, ...) do {       \
        if(!(expression))                      \
            ::Util::abort(F(__FILE__), __LINE__, ##__VA_ARGS__); \
    } while(false)
#else
    #define UTIL_ASSERT(expression) do { \
        if(!(expression))                \
            ::Util::abort();             \
    } while(false)
#endif

#define UTIL_LOGICAL_ERROR(...) UTIL_ASSERT(false, ##__VA_ARGS__)

namespace Util {

#if UTIL_VERBOSE_ASSERTS
    void abort(const __FlashStringHelper* file, int line, const __FlashStringHelper* error=nullptr) __attribute__((__noreturn__));
#else
    void abort() __attribute__((__noreturn__));
#endif

}

#endif
