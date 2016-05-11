#ifndef Util_Assertion_h
#define Util_Assertion_h

#define UTIL_ASSERT(expression) do { \
    if(!(expression))                \
        ::Util::abort();             \
} while(false)

namespace Util {

void abort() __attribute__((__noreturn__));

}

#endif

