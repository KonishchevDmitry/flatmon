#ifndef CO2Sensor_hpp
#define CO2Sensor_hpp

#include <AltSoftSerial.h>

#include <Util/Core.hpp>
#include <Util/CycleBuffer.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"

class CO2Sensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;
        typedef AltSoftSerial SensorSerial;

    private:
        enum class State: uint8_t;

    public:
        CO2Sensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    private:
        void onReadConcentration();
        void onReadingConcentration();
    /*
        void onTemperature(float temperature, float smoothedTemperature);
        void onComfortChange(Comfort comfort, bool initialChange);
    */

    private:
        SensorSerial* sensorSerial_;
        //Util::CycleBuffer<uint16_t, 10> values_;
        State state_;
        Comfort comfort_;
        byte response_[9];
        uint8_t receivedBytes_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Buzzer* buzzer_;
};

#endif
