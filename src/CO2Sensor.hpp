#ifndef CO2Sensor_hpp
#define CO2Sensor_hpp

// FIXME
#include <SoftwareSerial.h>
#include <AltSoftSerial.h>

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"

// Represents MH-Z19 CO2 sensor.
//
// Notice: Sensor automatically calibrates during a few days. During calibration it should be placed outdoor for some
// time to determine zero point.
class CO2Sensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;
        // FIXME
        // typedef AltSoftSerial SensorSerial;
        typedef SoftwareSerial SensorSerial;

    private:
        enum class State: uint8_t;

    public:
        CO2Sensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    private:
        void onReadConcentration();
        void onReadingConcentration();
        void onCommunicationError();
        void onComfort(Comfort comfort);

    private:
        SensorSerial* sensorSerial_;
        State state_;
        Comfort comfort_;
        byte response_[9];
        uint8_t receivedBytes_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Buzzer* buzzer_;
};

#endif
