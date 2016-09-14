#include "Assertion.hpp"
#include "Core.hpp"

#ifdef ARDUINO
    #include "Logging.hpp"
    using Util::Logging::log_critical;
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
                log_critical(F("Assertion error at "), file, F(":"), line, F(": "), error);
            else
                log_critical(F("Assertion error at "), file, F(":"), line, F("."));

            log_critical(F("Stopping the device."));

            if(ABORT_HANDLER)
                ABORT_HANDLER(file, line);
        #else
            log_critical(F("Assertion error. Stopping the device."));

            if(ABORT_HANDLER)
                ABORT_HANDLER();
        #endif

        Util::Core::stopDevice();
    #else
        ::abort();
    #endif
    }

    void setAbortHandler(AbortHandler handler) {
        ABORT_HANDLER = handler;
    }

}}
