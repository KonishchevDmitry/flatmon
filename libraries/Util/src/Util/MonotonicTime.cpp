#ifdef ARDUINO
    #include <Arduino.h>
#endif

#include "MonotonicTime.hpp"

namespace Util {

MonotonicTime::MonotonicTime(unsigned long epoch, unsigned long time)
: epoch_(epoch), time_(time) {
}

#ifdef ARDUINO
MonotonicTime MonotonicTime::now() {
    static MonotonicTime lastTime = MonotonicTime(0, 0);

    MonotonicTime curTime = MonotonicTime(lastTime.epoch_, millis());
    if(curTime.time_ < lastTime.time_)
        curTime.epoch_ += 1;

    return curTime;
}
#endif

bool MonotonicTime::operator==(const MonotonicTime& other) {
    return epoch_ == other.epoch_ && time_ == other.time_;
}

bool MonotonicTime::operator!=(const MonotonicTime& other) {
    return !(*this == other);
}

bool MonotonicTime::operator<=(const MonotonicTime& other) {
    return *this < other || *this == other;
}

bool MonotonicTime::operator>=(const MonotonicTime& other) {
    return *this > other || *this == other;
}

bool MonotonicTime::operator<(const MonotonicTime& other) {
    if(epoch_ < other.epoch_)
        return true;
    else if(epoch_ > other.epoch_)
        return false;
    else
        return time_ < other.time_;
}

bool MonotonicTime::operator>(const MonotonicTime& other) {
    if(epoch_ > other.epoch_)
        return true;
    else if(epoch_ < other.epoch_)
        return false;
    else
        return time_ > other.time_;
}

}

