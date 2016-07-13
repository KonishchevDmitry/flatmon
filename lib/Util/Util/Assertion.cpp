#include "Assertion.hpp"
#include "Core.hpp"

#ifdef ARDUINO
    #include "Logging.hpp"
    using Util::Logging::log;
#endif

namespace Util {

#if UTIL_VERBOSE_ASSERTS
    void abort(const __FlashStringHelper* file, int line, const __FlashStringHelper* error) {
#else
    void abort() {
#endif
    #ifdef ARDUINO
        #if UTIL_VERBOSE_ASSERTS
            if(error)
                log(F("Assertion error at "), file, F(":"), line, F(": "), error);
            else
                log(F("Assertion error at "), file, F(":"), line, F("."));

            log(F("Stopping the device."));
        #else
            log(F("Assertion error. Stopping the device."));
        #endif

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
