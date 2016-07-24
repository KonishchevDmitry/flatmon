#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Co2Sensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class Co2Sensor::Comfort: uint8_t {unknown, normal, warning, high, critical};
static const char* COMFORT_NAMES[] = {"unknown", "normal", "warning", "high", "critical"};
typedef Co2Sensor::Comfort Comfort;

#if CONFIG_ENABLE_CO2_SENSOR_PWM_EXPERIMENT
    extern volatile int CO2_SENSOR_PWM_VALUE;
    extern volatile int CO2_SENSOR_PWM_VALUE_STATUS;
    extern volatile TimeMicros CO2_SENSOR_PWM_DURATION;
#endif

namespace {
    // It looks like sensor measures CO2 concentration each ~ 6 seconds.
    // It shouldn't be polled more often then once in 10 seconds because in that case it returns strange results.
    const auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
    const auto PREHEAT_TIME = DEBUG_MODE ? POLLING_PERIOD : 3 * Constants::MINUTE_MILLIS;

    Comfort getComfort(uint16_t concentration) {
        // CO2 levels explained:
        // * 300 ppm – normal for village outdoor
        // * 500 ppm – normal for city outdoor
        // * 700-1500 ppm – normal for indoor:
        //   * 1000 ppm - decreased attention, initiative and level of perception
        //   * 1500 ppm - fatigue, hard to make decisions / work with information
        //   * 2000 ppm - apathy, headache, chronic fatigue syndrome

        uint16_t roundedConcentration = lround(float(concentration) / 100) * 100;

        if(roundedConcentration < 900)
            return Comfort::normal;
        else if(roundedConcentration < 1400)
            return Comfort::warning;
        else if(roundedConcentration < 1900)
            return Comfort::high;
        else
            return Comfort::critical;
    }
}

Co2Sensor::Co2Sensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: comfort_(Comfort::unknown), ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
    this->scheduleAfter(PREHEAT_TIME);
}

bool Co2Sensor::getConcentration(uint16_t *concentration) const {
    if(comfort_ == Comfort::unknown)
        return false;

    *concentration = concentration_;
    return true;
}

void Co2Sensor::onConcentration(uint16_t concentration) {
    concentration_ = concentration;

    Comfort comfort = getComfort(concentration_);
    log(F("CO2: "), concentration_, F(" ppm ("), COMFORT_NAMES[int(comfort)], F(")."));
    this->onComfort(comfort);
}

void Co2Sensor::onError() {
    this->onComfort(Comfort::unknown);
}

void Co2Sensor::onComfort(Comfort comfort) {
    if(comfort == comfort_)
        return;

    if(comfort == Comfort::unknown)
        ledProgress_.resume();
    else if(comfort_ == Comfort::unknown)
        ledProgress_.pause();
    else
        buzzer_->notify();

    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));
}


enum class Co2UartSensor::State: uint8_t {read, reading};

Co2UartSensor::Co2UartSensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: Co2Sensor(scheduler, ledGroup, buzzer), sensorSerial_(sensorSerial), state_(State::read) {
    sensorSerial_->begin(this->SERIAL_SPEED);
}

void Co2UartSensor::execute() {
    switch(state_) {
        case State::read:
            this->onReadConcentration();
            break;
        case State::reading:
            this->onReadingConcentration();
            break;
        default:
            UTIL_ASSERT(false);
            break;
    }
}


void Co2UartSensor::onReadConcentration() {
    #if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
        sensorSerial_->flushInput();
    #else
        while(sensorSerial_->read() != -1)
            ;
    #endif

    receivedBytes_ = 0;

    static byte getGasConcentrationCommand[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
    size_t sentBytes = sensorSerial_->write(getGasConcentrationCommand, sizeof getGasConcentrationCommand);
    UTIL_ASSERT(sentBytes == sizeof getGasConcentrationCommand);

    this->requestStartTime_ = millis();
    this->state_ = State::reading;

    constexpr size_t dataSize = sizeof getGasConcentrationCommand + sizeof response_;
    constexpr float dataSpeed = float(this->SERIAL_SPEED) / 8 / Constants::SECOND_MILLIS;
    constexpr TimeMillis minRequestTime = dataSize / dataSpeed;

    this->requestTimeout_ = 2 * minRequestTime;
    this->scheduleAfter(minRequestTime);
}

void Co2UartSensor::onReadingConcentration() {
    const int responseSize = sizeof response_;

    while(receivedBytes_ < responseSize) {
        int data = sensorSerial_->read();
        if(data == -1)
            break;

        response_[receivedBytes_++] = data;
    }

    if(receivedBytes_ < responseSize) {
        if(millis() - this->requestStartTime_ >= this->requestTimeout_) {
            log(F("CO2 sensor has timed out."));
            return this->onCommunicationError();
        }

        return this->scheduleAfter(1);
    }

    byte checksum = 0;
    for(int i = 1; i < responseSize - 1; i++)
        checksum += response_[i];
    checksum = byte(0xFF) - checksum + 1;

    if(response_[0] != 0xFF || response_[responseSize - 1] != checksum || response_[1] != 0x86) {
        log(F("CO2 sensor response validation error."));
        return this->onCommunicationError();
    }

    this->onConcentration(uint16_t(response_[2]) << 8 | response_[3]);

    #if CONFIG_ENABLE_CO2_SENSOR_PWM_EXPERIMENT
        log(">>> ", CO2_SENSOR_PWM_VALUE_STATUS, " ", CO2_SENSOR_PWM_DURATION, " ", CO2_SENSOR_PWM_VALUE);
    #endif

    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}

void Co2UartSensor::onCommunicationError() {
    this->onError();
    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}
