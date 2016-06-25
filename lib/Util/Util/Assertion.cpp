#include "Assertion.hpp"
#include "Core.hpp"

#ifdef ARDUINO
    #include "Logging.hpp"
    using Util::Logging::log;
#endif

namespace Util {

#if UTIL_VERBOSE_ASSERTS
    void abort(const char* file, int line) {
#else
    void abort() {
#endif
        #ifdef ARDUINO
            #if UTIL_VERBOSE_ASSERTS
                log(F("Assertion error at "), file, F(":"), line, F(". Stopping the device."));
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
