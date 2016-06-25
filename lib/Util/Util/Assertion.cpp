#include "Assertion.hpp"
#include "Core.hpp"

#ifdef ARDUINO
    #include "Logging.hpp"
    using Util::Logging::log;
#endif

namespace Util {

void abort() {
    #ifdef ARDUINO
        log(F("Assertion error. Stopping the device."));

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
