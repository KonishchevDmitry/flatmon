#ifndef PressureSensor_hpp
#define PressureSensor_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include <SFE_BMP180.h>

#include "Display.hpp"
#include "Indication.hpp"

class PressureSensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;

    private:
        enum class State: uint8_t;
        typedef void (PressureSensor::* StateHandler)();

    public:
        PressureSensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Display* display=nullptr);

    public:
        bool getPressure(uint16_t* pressure) const;
        virtual void execute();

    private:
        void onInitialize();
        void onStartTemperatureReading();
        void onReadTemperature();
        void onReadPressure();
        void onError(const FlashChar* error);
        void onComfort(Comfort comfort);

    private:
        static StateHandler stateHandlers_[];

        SFE_BMP180 barometer_;
        State state_;

        double temperature_;
        uint16_t pressure_;
        Comfort comfort_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Display* display_;
};

#endif
