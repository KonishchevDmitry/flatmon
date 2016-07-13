#ifndef Indication_hpp
#define Indication_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

struct ShiftRegisterLeds {
    public:
        typedef uint8_t LedsValue;
        static const uint8_t MAX_LEDS_NUM = sizeof(LedsValue) * 8;

    public:
        ShiftRegisterLeds(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);

    public:
        void set(LedsValue leds);
        void update(LedsValue leds, LedsValue mask);

    private:
        uint8_t dataPin_;
        uint8_t clockPin_;
        uint8_t latchPin_;
        LedsValue value_;
};

struct LedGroup {
    public:
        typedef ShiftRegisterLeds::LedsValue LedsValue;

    public:
        LedGroup(ShiftRegisterLeds* leds, uint8_t startBit, uint8_t ledsNum);

    public:
        void setLed(uint8_t ledNum);

    public:
        const uint8_t ledsNum;

    private:
        ShiftRegisterLeds* leds_;
        uint8_t startBit_;
        LedsValue mask_;
};

class LedProgressTask: public Util::Task {
    public:
        LedProgressTask(LedGroup* ledGroup);

    public:
        virtual void execute();

    private:
        LedGroup* ledGroup_;
        uint8_t curLedNum_;
};

template <size_t ledsNum>
class LedBrightnessRegulator: public Util::Task {
    public:
        LedBrightnessRegulator(uint8_t lightSensorPin, const uint8_t transistorBasePins[ledsNum],
                               Util::TaskScheduler* scheduler);

    public:
        virtual void execute();

    private:
        uint8_t lightSensorPin_;
        uint8_t transistorBasePins_[ledsNum];
    #if UTIL_ENABLE_LOGGING
        TimeMillis lastLogTime_;
    #endif
};

#include "Indication.tpp"

#endif
