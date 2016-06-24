#ifndef ARDUINO

#include "Core.hpp"
#include "Mocks.hpp"

namespace {
    TimeMillis CUR_TIME = 0;
}

unsigned long millis() {
    return CUR_TIME;
}

void delay(unsigned long time) {
    CUR_TIME += time;
}

namespace Util { namespace Mocks {

void setTime(TimeMillis time) {
    CUR_TIME = time;
}

}}

#endif
