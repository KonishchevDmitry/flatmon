#ifndef Util_Logging_hpp
#define Util_Logging_hpp

#include "Core.hpp"

#define UTIL_LOG_LEVEL_DISABLED 0
#define UTIL_LOG_LEVEL_CRITICAL 1
#define UTIL_LOG_LEVEL_ERROR    2
#define UTIL_LOG_LEVEL_WARNING  3
#define UTIL_LOG_LEVEL_INFO     4
#define UTIL_LOG_LEVEL_DEBUG    5

#ifndef UTIL_LOG_LEVEL
    #define UTIL_LOG_LEVEL UTIL_LOG_LEVEL_DISABLED
#endif

#ifndef UTIL_LOG_ENABLE_TIMESTAMPS
    #define UTIL_LOG_ENABLE_TIMESTAMPS (UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_DEBUG)
#endif

namespace Util { namespace Logging {

inline void init(unsigned long speed = 9600) {
#if UTIL_LOG_LEVEL != UTIL_LOG_LEVEL_DISABLED
    Serial.begin(speed);
#endif
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

namespace aux {
    template <typename T>
    void log_args(T arg) {
    #if UTIL_LOG_LEVEL != UTIL_LOG_LEVEL_DISABLED
        Serial.println(arg);
    #endif
    }

    template <typename T, typename... Args>
    void log_args(T arg, Args... args) {
    #if UTIL_LOG_LEVEL != UTIL_LOG_LEVEL_DISABLED
        Serial.print(arg);
        log_args(args...);
    #endif
    }

    template <typename... Args>
    void log(Args... args) {
        log_args(
        #if UTIL_LOG_ENABLE_TIMESTAMPS
            millis(), F("> "),
        #endif
            args...
        );
    }
}

template <typename... Args>
void log_critical(Args... args) {
#if UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_CRITICAL
    aux::log(F("C: "), args...);
#endif
}

template <typename... Args>
void log_error(Args... args) {
#if UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_ERROR
    aux::log(F("E: "), args...);
#endif
}

template <typename... Args>
void log_warning(Args... args) {
#if UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_WARNING
    aux::log(F("W: "), args...);
#endif
}

template <typename... Args>
void log_info(Args... args) {
#if UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_INFO
    aux::log(F("I: "), args...);
#endif
}

template <typename... Args>
void log_debug(Args... args) {
#if UTIL_LOG_LEVEL >= UTIL_LOG_LEVEL_DEBUG
    aux::log(F("D: "), args...);
#endif
}

#pragma GCC diagnostic pop

}}

#endif
