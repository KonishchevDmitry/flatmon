#ifdef ARDUINO

#include <avr/wdt.h>

#include "Assertion.hpp"
#include "Core.hpp"
#include "Logging.hpp"

using Util::Logging::log_debug;

extern int* __brkval;
extern int __heap_start;

namespace Util { namespace Core {

void init() {
    // On some boards built in LED is on by default. Turn if off.
    registerUsedPin(LED_BUILTIN);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, false);
}

size_t getStackFreeMemorySize() {
    char var;
    auto stackAddress = reinterpret_cast<uintptr_t>(&var);
    auto heapAddress = reinterpret_cast<uintptr_t>(__brkval ? __brkval : &__heap_start);
    return heapAddress < stackAddress ? stackAddress - heapAddress : 0;
}

void stopDevice() {
    wdt_disable();

    bool state = false;
    pinMode(LED_BUILTIN, OUTPUT);

    while(true) {
        state = !state;
        digitalWrite(LED_BUILTIN, state);
        delay(500);
    }
}


namespace {
    uint8_t USED_PINS[NUM_DIGITAL_PINS / 8 + bool(NUM_DIGITAL_PINS % 8)];

    uint8_t& getPinGroup(uint8_t pin) {
        return USED_PINS[pin / 8];
    }

    uint8_t getPinMask(uint8_t pin) {
        return 1 << (pin % 8);
    }
}

void registerUsedPin(uint8_t pin) {
    UTIL_ASSERT(pin < NUM_DIGITAL_PINS);

    uint8_t& pinGroup = getPinGroup(pin);
    uint8_t pinMask = getPinMask(pin);

    UTIL_ASSERT(!(pinGroup & pinMask), F("The pin is already used."));
    pinGroup |= pinMask;
}

void configureUnusedPins() {
    // Unconnected digital inputs can saturate the input inverters and cause high-current flows that cause chip
    // overheat, burnout, or I/O port function disruption.
    //
    // Unconnected analog inputs can trigger input MUX circuits ON and badly distort signals from other analog inputs.

    log_debug(F("Configuring the following unused pins:"));
    for(uint8_t pin = 0; pin < NUM_DIGITAL_PINS; pin++) {
        if(!(getPinGroup(pin) & getPinMask(pin))) {
            log_debug(F("* "), pin);
            // FIXME
        }
    }
}

}}

#endif
