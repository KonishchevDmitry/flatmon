#ifndef Util_Timing_hpp
#define Util_Timing_hpp

#include "Core.hpp"
#include "Logging.hpp"

namespace Util {

using Logging::log_critical;

class ScopedTiming {
    public:
        ScopedTiming(const char* name): name_(name), startTime_(micros()) {
        }

        ~ScopedTiming() {
            TimeMicros executionTime = micros() - startTime_;
            log_critical(name_, F(" execution time: "), executionTime, F("."));
        }

    private:
        const char* name_;
        TimeMicros startTime_;
};

}

#endif
