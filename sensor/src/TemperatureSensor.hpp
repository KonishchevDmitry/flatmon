#ifndef TemperatureSensor_hpp
#define TemperatureSensor_hpp

#include <Util/Core.hpp>
#include <Util/CycleBuffer.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"

// FIXME: Deprecate by DHT22 (don't drop until we'll be sure that we don't need cycle buffer in DHT22)
class TemperatureSensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;

    public:
        TemperatureSensor(uint8_t sensorPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup);

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
