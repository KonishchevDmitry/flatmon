#ifdef ARDUINO

#include <avr/wdt.h>

#include "Core.hpp"

extern int* __brkval;
extern int __heap_start;

namespace Util { namespace Core {

void init() {
    // On some boards built in LED is on by default. Turn if off.
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

}}

#endif
