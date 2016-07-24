#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Co2Sensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class Co2Sensor::Comfort: uint8_t {unknown, normal, warning, high, critical};
const char* Co2Sensor::COMFORT_NAMES_[] = {"unknown", "normal", "warning", "high", "critical"};

typedef Co2Sensor::Comfort Comfort;
typedef Co2Sensor::Concentration Concentration;

namespace {
    // It looks like sensor measures CO2 concentration each ~ 6 seconds.
    // It shouldn't be polled more often then once in 10 seconds via UART because in that case it returns strange
    // results.
    const auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
    const auto PREHEAT_TIME = DEBUG_MODE ? POLLING_PERIOD : 3 * Constants::MINUTE_MILLIS;

    Comfort getComfort(Concentration concentration) {
        // CO2 levels explained:
        // * 300 ppm – normal for village outdoor
        // * 500 ppm – normal for city outdoor
        // * 700-1500 ppm – normal for indoor:
        //   * 1000 ppm - decreased attention, initiative and level of perception
        //   * 1500 ppm - fatigue, hard to make decisions / work with information
        //   * 2000 ppm - apathy, headache, chronic fatigue syndrome

        Concentration roundedConcentration = lround(float(concentration) / 100) * 100;

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

bool Co2Sensor::getConcentration(Concentration *concentration) const {
    if(comfort_ == Comfort::unknown)
        return false;

    *concentration = concentration_;
    return true;
}

void Co2Sensor::onConcentration(Concentration concentration) {
    concentration_ = concentration;

    Comfort comfort = getComfort(concentration_);
    log(F("CO2: "), concentration_, F(" ppm ("), COMFORT_NAMES_[int(comfort)], F(")."));
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


enum class Co2PwmSensor::Status: uint8_t {no_data, ok, bounce_error, timing_error, logical_error};
const char* Co2PwmSensor::STATUS_NAMES_[] = {"no-data", "ok", "bounce-error", "timing-error", "logical-error"};

constexpr uint16_t PWM_PIN_UNSET = 0xFFFF;
uint16_t Co2PwmSensor::PWM_PIN_ = PWM_PIN_UNSET;

Co2PwmSensor::State Co2PwmSensor::STATE_ = Co2PwmSensor::State::initializing;
volatile Co2PwmSensor::Status Co2PwmSensor::STATUS_ = Co2PwmSensor::Status::no_data;
volatile Concentration Co2PwmSensor::CONCENTRATION_;

TimeMicros Co2PwmSensor::CYCLE_START_TIME_;
TimeMicros Co2PwmSensor::LOW_LEVEL_START_TIME_;

void Co2PwmSensor::init(uint8_t pwmPin) {
    if(PWM_PIN_ != PWM_PIN_UNSET) {
        UTIL_ASSERT(pwmPin == PWM_PIN_);
        return;
    }

    PWM_PIN_ = pwmPin;
    pinMode(pwmPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pwmPin), Co2PwmSensor::onPwmValueChanged, CHANGE);
}

void Co2PwmSensor::onPwmValueChanged() {
    switch(STATE_) {
        case State::initializing:
            if(digitalRead(PWM_PIN_)) {
                CYCLE_START_TIME_ = micros();
                STATE_ = State::high_level;
            }
            break;

        case State::high_level:
            if(digitalRead(PWM_PIN_)) {
                STATUS_ = Status::bounce_error;
                STATE_ = State::initializing;
            } else {
                LOW_LEVEL_START_TIME_ = micros();
                STATE_ = State::low_level;
            }
            break;

        case State::low_level: {
            if(!digitalRead(PWM_PIN_)) {
                STATUS_ = Status::bounce_error;
                STATE_ = State::initializing;
                break;
            }

            TimeMicros curTime = micros();
            TimeMicros curCycleDuration = curTime - CYCLE_START_TIME_;
            TimeMicros curHighLevelDuration = LOW_LEVEL_START_TIME_ - CYCLE_START_TIME_;
            TimeMicros curLowLevelDuration = curTime - LOW_LEVEL_START_TIME_;

            constexpr Concentration maxPpm = 5000;
            constexpr TimeMicros highLevelMarkDuration = 2000;
            constexpr TimeMicros lowLevelMarkDuration = 2000;
            constexpr TimeMicros dataDuration = 1000000L;
            constexpr TimeMicros cycleDuration = highLevelMarkDuration + dataDuration + lowLevelMarkDuration;
            constexpr TimeMicros minPrecision = dataDuration / maxPpm / 2;

            if(
                curCycleDuration < cycleDuration - minPrecision ||
                curCycleDuration > cycleDuration + minPrecision ||
                curHighLevelDuration < highLevelMarkDuration - minPrecision ||
                curLowLevelDuration < lowLevelMarkDuration - minPrecision
            ) {
                STATUS_ = Status::timing_error;
                STATE_ = State::initializing;
                break;
            }

            TimeMicros curDataDuration = curHighLevelDuration > highLevelMarkDuration
                ? curHighLevelDuration - highLevelMarkDuration
                : 0;

            // Since we know all boundary values, use decimal arithmetic here instead of floating point arithmetic to
            // not spend too much CPU cycles in ISR.
            CONCENTRATION_ = (curDataDuration / 10) * maxPpm / (dataDuration / 10);
            STATUS_ = Status::ok;

            CYCLE_START_TIME_ = curTime;
            STATE_ = State::high_level;

            break;
        }

        default:
            STATE_ = State::initializing;
            STATUS_ = Status::logical_error;
            break;
    }
}

void Co2PwmSensor::acquireCurrentStatus(Status* status, Concentration* concentration) {
    noInterrupts();
    *status = STATUS_;
    *concentration = CONCENTRATION_;
    STATUS_ = Status::no_data;
    interrupts();
}

Co2PwmSensor::Co2PwmSensor(uint8_t pwmPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: Co2Sensor(scheduler, ledGroup, buzzer) {
    this->init(pwmPin);
}

void Co2PwmSensor::execute() {
    Status status;
    Concentration concentration;
    this->acquireCurrentStatus(&status, &concentration);

    if(status == Status::ok) {
        this->onConcentration(concentration);
    } else {
        log(F("Error while reading CO2 concentration using PWM: "), STATUS_NAMES_[int(status)], F("."));
        this->onError();
    }

    this->scheduleAfter(POLLING_PERIOD);
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

    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}

void Co2UartSensor::onCommunicationError() {
    this->onError();
    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}
