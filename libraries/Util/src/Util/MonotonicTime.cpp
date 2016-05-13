#ifdef ARDUINO
    #include <Arduino.h>
#else
    #include "Mocks.hpp"
#endif

#include "MonotonicTime.hpp"

namespace Util {

MonotonicTime::MonotonicTime()
: MonotonicTime(0, 0) {
}

MonotonicTime::MonotonicTime(Epoch epoch, Time time)
: epoch(epoch), time(time) {
}

MonotonicTime MonotonicTime::now() {
    static MonotonicTime lastTime = MonotonicTime(0, 0);

    MonotonicTime curTime = MonotonicTime(lastTime.epoch, millis());
    if(curTime.time < lastTime.time)
        curTime.epoch += 1;

    return curTime;
}

MonotonicTime& MonotonicTime::add(Time time) {
    Time prevTime = this->time;

    this->time += time;
    if(this->time < prevTime)
        this->epoch += 1;

    return *this;
}

bool MonotonicTime::operator==(const MonotonicTime& other) const {
    return this->epoch == other.epoch && this->time == other.time;
}

bool MonotonicTime::operator!=(const MonotonicTime& other) const {
    return !(*this == other);
}

bool MonotonicTime::operator<=(const MonotonicTime& other) const {
    return *this < other || *this == other;
}

bool MonotonicTime::operator>=(const MonotonicTime& other) const {
    return *this > other || *this == other;
}

bool MonotonicTime::operator<(const MonotonicTime& other) const {
    if(this->epoch < other.epoch)
        return true;
    else if(this->epoch > other.epoch)
        return false;
    else
        return this->time < other.time;
}

bool MonotonicTime::operator>(const MonotonicTime& other) const {
    if(this->epoch > other.epoch)
        return true;
    else if(this->epoch < other.epoch)
        return false;
    else
        return this->time > other.time;
}

MonotonicTime MonotonicTime::operator+(Time time) const {
    auto newTime = *this;
    newTime += time;
    return newTime;
}

void MonotonicTime::operator+=(Time time) {
    this->add(time);
}

#ifndef ARDUINO
std::ostream& operator<<(std::ostream& os, const MonotonicTime& time) {
    return os << "MonotonicTime{epoch: " << time.epoch << ", time: " << time.time << "}";
}
#endif

}
