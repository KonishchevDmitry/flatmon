#ifndef Util_MonotonicTime_hpp
#define Util_MonotonicTime_hpp

#include "Core.hpp"

#ifndef ARDUINO
#import <ostream>
#endif

namespace Util {

class MonotonicTime {
    public:
        typedef uint8_t Epoch;
        typedef TimeMillis Time;

    public:
        MonotonicTime();
        MonotonicTime(Time time);
        MonotonicTime(Epoch epoch, Time time);

        static MonotonicTime now();

        bool operator==(const MonotonicTime& other) const;
        bool operator!=(const MonotonicTime& other) const;

        bool operator<=(const MonotonicTime& other) const;
        bool operator>=(const MonotonicTime& other) const;

        bool operator<(const MonotonicTime& other) const;
        bool operator>(const MonotonicTime& other) const;

        MonotonicTime operator+(MonotonicTime time) const;
        MonotonicTime operator-(MonotonicTime time) const;

        MonotonicTime& operator+=(MonotonicTime time);
        MonotonicTime& operator-=(MonotonicTime time);

    public:
        Epoch epoch;
        Time time;
};

#ifndef ARDUINO
std::ostream& operator<<(std::ostream& os, const MonotonicTime& time);
#endif

}

#endif
