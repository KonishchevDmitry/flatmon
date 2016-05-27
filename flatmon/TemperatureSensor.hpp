#ifndef TemperatureSensor_hpp
#define TemperatureSensor_hpp

#include <Arduino.h>

#include <Util/CycleBuffer.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"

class TemperatureSensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;

    public:
        TemperatureSensor(uint8_t sensorPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    private:
        void onTemperature(float temperature, float smoothedTemperature);
        void onComfortChange(Comfort comfort, bool initialChange);

    private:
        uint8_t sensorPin_;
        Util::CycleBuffer<uint16_t, 10> values_;
        Comfort comfort_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Buzzer* buzzer_;
};

#endif
