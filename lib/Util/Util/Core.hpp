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

#define UTIL_ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

#ifdef ARDUINO
    // A pointer returned by F() macro.
    typedef __FlashStringHelper FlashChar;
#else
    typedef char FlashChar;
    #define F(string) (string)
#endif

namespace Util { namespace Core {
    void init();
    size_t getStackFreeMemorySize();
    void stopDevice() __attribute__((__noreturn__));

    void registerUsedPin(uint8_t pin);

    template <typename T>
    void registerUsedPins(T pin) {
        registerUsedPin(pin);
    }

    template <typename T, typename... Pins>
    void registerUsedPins(T pin, Pins... pins) {
        registerUsedPin(pin);
        registerUsedPins(pins...);
    }

    void configureUnusedPins();
}}

#endif
