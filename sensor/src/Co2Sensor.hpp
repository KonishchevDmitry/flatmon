#ifndef Co2Sensor_hpp
#define Co2Sensor_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Config.hpp"
#include "Indication.hpp"

#if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
    #include <AltSoftSerial.h>
#endif

// Represents MH-Z19 CO2 sensor.
//
// Notice: Sensor automatically calibrates during a few days. During calibration it should be placed outdoor for some
// time to determine zero point.
class Co2Sensor: public Util::Task {
    public:
        enum class Comfort: uint8_t;

    public:
        Co2Sensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        bool getConcentration(uint16_t* concentration) const;

    protected:
        void onConcentration(uint16_t concentration);
        void onError();

    private:
        void onComfort(Comfort comfort);

    private:
        Comfort comfort_;
        uint16_t concentration_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Buzzer* buzzer_;
};

// FIXME: Consider to use PWM output for sensor reading to free up UART ports
class Co2UartSensor: public Co2Sensor {
    public:
        #if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
            typedef AltSoftSerial SensorSerial;
        #else
            typedef HardwareSerial SensorSerial;
        #endif

    private:
        enum class State: uint8_t;
        static constexpr int SERIAL_SPEED = 9600;

    public:
        Co2UartSensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer);

    public:
        virtual void execute();

    private:
        void onReadConcentration();
        void onReadingConcentration();
        void onCommunicationError();

    private:
        SensorSerial* sensorSerial_;
        State state_;

        byte response_[9];
        uint8_t receivedBytes_;
        TimeMillis requestStartTime_;
        TimeMillis requestTimeout_;
};

#endif
