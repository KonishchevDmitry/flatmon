// This file should be included in each source file to provide the basic and properly configured functionality for your
// project.

#ifndef Util_Core_hpp
#define Util_Core_hpp

#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include <cstddef>
    #include <cstdlib>
#endif

// Type which is used to store time in milliseconds.
typedef unsigned long TimeMillis;

// Type which is used to store time in microseconds.
typedef unsigned long TimeMicros;

#endif
