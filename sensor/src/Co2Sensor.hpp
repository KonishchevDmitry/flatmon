#ifndef Co2Sensor_hpp
#define Co2Sensor_hpp

#include <Util/Core.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Config.hpp"
#include "Display.hpp"
#include "Indication.hpp"

#define CO2_PWM_SENSOR_ENABLE_PROFILING 0

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
        typedef uint16_t Concentration;

    public:
        Co2Sensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Display* display=nullptr, Buzzer* buzzer=nullptr);

    public:
        bool getConcentration(Concentration* concentration) const;

    protected:
        virtual const char* getName() = 0;
        void onConcentration(Concentration concentration);
        void onError();

    private:
        void onComfort(Comfort comfort);

    private:
        static const char* COMFORT_NAMES_[];

        Comfort comfort_;
        uint16_t concentration_;

        LedGroup* ledGroup_;
        LedProgressTask ledProgress_;
        Display* display_;
        Buzzer* buzzer_;
};

class Co2PwmSensor: public Co2Sensor {
    private:
        enum class State: uint8_t {initializing, high_level, low_level};
        enum class Status: uint8_t;

    public:
        Co2PwmSensor(uint8_t pwmPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup,
                     Display* display=nullptr, Buzzer* buzzer=nullptr);

    public:
    #if CO2_PWM_SENSOR_ENABLE_PROFILING
        static void profile(uint8_t pwmPin);
    #endif
        virtual void execute();

    protected:
        virtual const char* getName();

    private:
        static void init(uint8_t pwmPin);
        static void onPwmValueChanged();
        static void acquireCurrentStatus(Status* status, Concentration* concentration);

    private:
        static const char* STATUS_NAMES_[];

        static uint16_t PWM_PIN_;

        static State STATE_;
        static TimeMicros CYCLE_START_TIME_;
        static TimeMicros LOW_LEVEL_START_TIME_;

        static volatile Status STATUS_;
        static volatile Concentration CONCENTRATION_;
};

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
        Co2UartSensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup,
                      Display* display=nullptr, Buzzer* buzzer=nullptr);

    public:
        virtual void execute();

    protected:
        virtual const char* getName();

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
