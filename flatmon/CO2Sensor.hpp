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

    public:
        CO2Sensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    /*
    private:
        void onTemperature(float temperature, float smoothedTemperature);
        void onComfortChange(Comfort comfort, bool initialChange);
    */

    private:
        SensorSerial* sensorSerial_;
        //uint8_t sensorPin_;
        Util::CycleBuffer<uint16_t, 10> values_;
        Comfort comfort_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Buzzer* buzzer_;
};

#endif
