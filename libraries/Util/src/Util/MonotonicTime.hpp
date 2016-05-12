#ifndef Util_MonotonicTime_hpp
#define Util_MonotonicTime_hpp

namespace Util {

class MonotonicTime {
    public:
        MonotonicTime(unsigned long epoch, unsigned long time);

        static MonotonicTime now();

        bool operator==(const MonotonicTime& other);
        bool operator!=(const MonotonicTime& other);

        bool operator<=(const MonotonicTime& other);
        bool operator>=(const MonotonicTime& other);

        bool operator<(const MonotonicTime& other);
        bool operator>(const MonotonicTime& other);

    private:
        unsigned long epoch_;
        unsigned long time_;
};

}

#endif
