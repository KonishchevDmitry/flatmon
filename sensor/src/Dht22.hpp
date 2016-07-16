#ifndef Dht22_hpp
#define Dht22_hpp

#include <Util/Core.hpp>
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
        bool getHumidity(uint8_t* humidity);
        bool getTemperature(int8_t* temperature);

        virtual void execute();

    private:
        void onStartReading();
        void onReading();
        void onError();

        void onHumidityComfort(HumidityComfort comfort);
        void onTemperatureComfort(TemperatureComfort comfort);

        bool receiveData(uint16_t* data, uint8_t size);
        bool waitForLogicLevel(bool level, TimeMicros timeout);
        void stopReading();

    private:
        uint8_t dataPin_;
        State state_;

        uint8_t humidity_;
        HumidityComfort humidityComfort_;
        LedGroup* humidityLedGroup_;
        LedProgressTask humidityLedProgress_;

        int8_t temperature_;
        TemperatureComfort temperatureComfort_;
        LedGroup* temperatureLedGroup_;
        LedProgressTask temperatureLedProgress_;

        Buzzer* buzzer_;
};

#endif
