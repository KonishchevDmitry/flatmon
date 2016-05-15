#ifndef Temperature_hpp
#define Temperature_hpp

#include <Arduino.h>

#include <Util/CycleBuffer.hpp>
#include <Util/TaskScheduler.hpp>

#include "Indication.hpp"

class TemperatureSensor: public Util::Task {
    public:
        // Attention: Be careful when changing the enum variants: they are implicitly attached to LEDs and comfort name
        // mapping logic.
        enum class Comfort: uint8_t {UNKNOWN, COLD, NORMAL, WARM, HOT};

    public:
        TemperatureSensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, uint8_t sensorPin);

    public:
        virtual void execute();

    private:
        void onTemperature(float temperature);

    private:
        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;

        uint8_t sensorPin_;
        Util::CycleBuffer<uint16_t, 10> values_;
        Comfort comfort_;
};

#endif
