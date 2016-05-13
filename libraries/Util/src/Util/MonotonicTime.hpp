#ifndef Util_MonotonicTime_hpp
#define Util_MonotonicTime_hpp

#ifndef ARDUINO
#import <ostream>
#endif

namespace Util {

class MonotonicTime {
    public:
        typedef unsigned long Epoch;
        typedef unsigned long Time;

    public:
        MonotonicTime();
        MonotonicTime(Epoch epoch, Time time);

        static MonotonicTime now();
        MonotonicTime& add(Time time);

        bool operator==(const MonotonicTime& other) const;
        bool operator!=(const MonotonicTime& other) const;

        bool operator<=(const MonotonicTime& other) const;
        bool operator>=(const MonotonicTime& other) const;

        bool operator<(const MonotonicTime& other) const;
        bool operator>(const MonotonicTime& other) const;

        MonotonicTime operator+(Time time) const;
        void operator+=(Time time);

    public:
        Epoch epoch;
        Time time;
};

#ifndef ARDUINO
std::ostream& operator<<(std::ostream& os, const MonotonicTime& time);
#endif

}

#endif
