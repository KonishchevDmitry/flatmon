#ifndef ARDUINO
#include "Mocks.hpp"

namespace {
    unsigned long CUR_TIME = 0;
}

unsigned long millis() {
    return CUR_TIME;
}

void delay(unsigned long time) {
    CUR_TIME += time;
}

namespace Util { namespace Mocks {

void setTime(unsigned long time) {
    CUR_TIME = time;
}

}}

#endif
