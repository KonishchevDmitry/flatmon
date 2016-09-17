#ifndef Dht22_hpp
#define Dht22_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Display.hpp"
#include "Indication.hpp"

class Dht22: public Util::Task {
    public:
        enum class TemperatureComfort: uint8_t;
        enum class HumidityComfort: uint8_t;

    private:
        enum class State: uint8_t;
        typedef void (Dht22::* StateHandler)();

    public:
        Dht22(uint8_t dataPin, Util::TaskScheduler* scheduler,
              LedGroup* temperatureLedGroup, LedGroup* humidityLedGroup,
              Display* display = nullptr);

    public:
        bool getTemperature(int8_t* temperature) const;
        bool getHumidity(uint8_t* humidity) const;

        virtual const FlashChar* getName() { return F("DHT22 sensor"); }
        virtual void execute();

    private:
        void onStartReading();
        void onReading();
        void onError();

        void onTemperatureComfort(TemperatureComfort comfort);
        void onHumidityComfort(HumidityComfort comfort);

        bool receiveData(uint16_t* data, uint8_t size);
        bool waitForLogicLevel(bool level, TimeMicros timeout);
        void stopReading();

    private:
        static StateHandler stateHandlers_[];

        uint8_t dataPin_;
        State state_;

        int8_t temperature_;
        TemperatureComfort temperatureComfort_;
        LedGroup* temperatureLedGroup_;
        LedProgressTask temperatureLedProgress_;

        uint8_t humidity_;
        HumidityComfort humidityComfort_;
        LedGroup* humidityLedGroup_;
        LedProgressTask humidityLedProgress_;

        Display* display_;
};

#endif
