#ifndef Dht22_hpp
#define Dht22_hpp

#include <Util/Core.hpp>
#include <Util/CycleBuffer.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"

class Dht22: public Util::Task {
    public:
        enum class HumidityComfort: uint8_t;
        enum class TemperatureComfort: uint8_t;

    private:
        enum class State: uint8_t;

    public:
        Dht22(uint8_t dataPin, Util::TaskScheduler* scheduler,
              LedGroup* humidityLedGroup, LedGroup* temperatureLedGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    private:
        void onStartReading();
        void onReading();
        void onError();

        bool waitForLogicLevel(bool level, TimeMicros timeout);

        // FIXME
        void onTemperature(float temperature, float smoothedTemperature);
        void onComfortChange(TemperatureComfort comfort, bool initialChange);

    private:
        uint8_t dataPin_;
        State state_;

        HumidityComfort humidityComfort_;
        LedGroup* humidityLedGroup_;
        LedProgressTask humidityLedProgress_;

        TemperatureComfort temperatureComfort_;
        LedGroup* temperatureLedGroup_;
        LedProgressTask temperatureLedProgress_;

        Buzzer* buzzer_;
};

#endif
