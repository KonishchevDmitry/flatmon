#ifndef Indication_hpp
#define Indication_hpp

#include <Arduino.h>

struct ShiftRegisterLeds {
    public:
        typedef uint8_t LedsValue;
        static const uint8_t MAX_LEDS_NUM = sizeof(LedsValue) * 8;

    public:
        ShiftRegisterLeds(uint8_t dataPin, uint8_t clockPin, uint8_t latchPin);

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
        LedGroup(ShiftRegisterLeds* leds, uint8_t startBit, uint8_t ledNum);

        void setLed(uint8_t ledNum);

    private:
        ShiftRegisterLeds* leds_;
        uint8_t startBit_;
        uint8_t ledNum_;
        LedsValue mask_;
};

#endif
