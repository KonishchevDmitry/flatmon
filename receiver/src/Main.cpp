#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Logging.hpp>

using Util::Logging::log;

void setup() {
    Util::Logging::init();
    log(F("Initializing..."));
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
