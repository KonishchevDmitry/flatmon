#ifndef Util_Assertion_hpp
#define Util_Assertion_hpp

#include "Core.hpp"

#if UTIL_VERBOSE_ASSERTS
    #define UTIL_ASSERT(expression, ...) do {                    \
        if(!(expression))                                        \
            ::Util::Assertion::abort(F(__FILE__), __LINE__, ##__VA_ARGS__); \
    } while(false)
#else
    #define UTIL_ASSERT(expression, ...) do { \
        if(!(expression))                \
            ::Util::Assertion::abort();             \
    } while(false)
#endif

#define UTIL_LOGICAL_ERROR(...) UTIL_ASSERT(false, ##__VA_ARGS__)

namespace Util { namespace Assertion {

#if UTIL_VERBOSE_ASSERTS
    typedef void (*AbortHandler)(const FlashString* file, int line);
    void abort(const FlashString* file, int line, const FlashString* error=nullptr) __attribute__((__noreturn__));
#else
    typedef void (*AbortHandler)();
    void abort() __attribute__((__noreturn__));
#endif

void setAbortHandler(AbortHandler handler);

}}

#endif
