#ifndef Util_Logging_hpp
#define Util_Logging_hpp

#ifndef UTIL_ENABLE_LOGGING
    #error UTIL_ENABLE_LOGGING macro must be defined to use logging module.
#endif

#include <Arduino.h>

namespace Util { namespace Logging {

inline void init() {
#if UTIL_ENABLE_LOGGING
    Serial.begin(9600);
#endif
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

template <typename A1>
void log(A1 a1) {
#if UTIL_ENABLE_LOGGING
    Serial.println(a1);
#endif
}

template <typename A1, typename A2>
void log(A1 a1, A2 a2) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2);
#endif
}

template <typename A1, typename A2, typename A3>
void log(A1 a1, A2 a2, A3 a3) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3);
#endif
}

template <typename A1, typename A2, typename A3, typename A4>
void log(A1 a1, A2 a2, A3 a3, A4 a4) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5, a6);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5, a6, a7);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5, a6, a7, a8);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5, a6, a7, a8, a9);
#endif
}

template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
void log(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) {
#if UTIL_ENABLE_LOGGING
    Serial.print(a1);
    log(a2, a3, a4, a5, a6, a7, a8, a9, a10);
#endif
}

#pragma GCC diagnostic pop

}}

#endif
