#ifndef Util_Logging_hpp
#define Util_Logging_hpp

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

template <typename T, typename... Args>
void vlog(T arg, Args... args) {
#if UTIL_ENABLE_LOGGING && UTIL_LOG_VERBOSITY > 0
    log(arg, args...);
#endif
}

template <typename T, typename... Args>
void vvlog(T arg, Args... args) {
#if UTIL_ENABLE_LOGGING && UTIL_LOG_VERBOSITY > 1
    log(arg, args...);
#endif
}

#pragma GCC diagnostic pop

}}

#endif
