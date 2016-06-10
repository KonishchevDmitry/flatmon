#ifndef Util_Logging_hpp
#define Util_Logging_hpp

#ifndef UTIL_ENABLE_LOGGING
    #error UTIL_ENABLE_LOGGING macro must be defined to use logging module.
#endif

#include "Core.hpp"

namespace Util { namespace Logging {

inline void init() {
#if UTIL_ENABLE_LOGGING
    Serial.begin(9600);
#endif
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

template <typename T>
void log(T arg) {
#if UTIL_ENABLE_LOGGING
    Serial.println(arg);
#endif
}

template <typename T, typename... Args>
void log(T arg, Args... args) {
#if UTIL_ENABLE_LOGGING
    Serial.print(arg);
    log(args...);
#endif
}

#pragma GCC diagnostic pop

}}

#endif
