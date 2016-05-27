#include "Assertion.hpp"
#include "Core.hpp"

namespace Util {

void abort() {
    #ifdef ARDUINO
        bool state = false;
        pinMode(LED_BUILTIN, OUTPUT);

        while(true) {
            state = !state;
            digitalWrite(LED_BUILTIN, state);
            delay(500);
        }
    #else
        ::abort();
    #endif
}

}
