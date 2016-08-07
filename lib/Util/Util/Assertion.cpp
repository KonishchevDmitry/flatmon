#include "Assertion.hpp"
#include "Core.hpp"

#ifdef ARDUINO
    #include "Logging.hpp"
    using Util::Logging::log;
#endif

namespace Util { namespace Assertion {

    namespace {
        AbortHandler ABORT_HANDLER = nullptr;
    }

#if UTIL_VERBOSE_ASSERTS
    void abort(const FlashChar* file, int line, const FlashChar* error) {
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

            if(ABORT_HANDLER)
                ABORT_HANDLER(file, line);
        #else
            log(F("Assertion error. Stopping the device."));

            if(ABORT_HANDLER)
                ABORT_HANDLER();
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

    void setAbortHandler(AbortHandler handler) {
        ABORT_HANDLER = handler;
    }

}}
