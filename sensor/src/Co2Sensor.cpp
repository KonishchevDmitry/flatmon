#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Co2Sensor.hpp"
#include "Config.hpp"

using Util::Logging::log_error;
using Util::Logging::log_info;
using Util::Logging::log_debug;
using Util::MonotonicTime;
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
    const auto PREHEAT_TIME = CONFIG_DEVELOP_MODE ? POLLING_PERIOD : 3 * Constants::MINUTE_MILLIS;

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

Co2Sensor::Co2Sensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Display* display, Buzzer* buzzer)
: comfort_(Comfort::unknown), ledGroup_(ledGroup), ledProgress_(ledGroup), display_(display), buzzer_(buzzer) {
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
    log_info(F("CO2 ("), this->getMode(), F("): "), concentration_, F(" ppm ("), COMFORT_NAMES_[int(comfort)], F(")."));

    if(display_)
        display_->setCo2Concentration(concentration);

    this->onComfort(comfort);
}

void Co2Sensor::onError() {
    if(display_)
        display_->resetCo2Concentration();

    this->onComfort(Comfort::unknown);
}

void Co2Sensor::onComfort(Comfort comfort) {
    bool changed = comfort != comfort_;

    if(comfort == Comfort::unknown) {
        if(changed) {
            comfort_ = comfort;
            warningLevelStartTime_ = MonotonicTime();
            ledProgress_.resume();
        }
        return;
    }

    if(comfort_ == Comfort::unknown)
        ledProgress_.pause();

    if(buzzer_) {
        MonotonicTime noTime = MonotonicTime();
        MonotonicTime curTime = MonotonicTime::now();
        bool onWarningLevel = uint8_t(comfort) >= uint8_t(Comfort::warning);

        if(onWarningLevel) {
            if(warningLevelStartTime_ == noTime)
                warningLevelStartTime_ = curTime;
            else if(
                curTime - warningLevelStartTime_ >= 5 * Constants::MINUTE_MILLIS &&
                (lastNotificationTime_ == noTime || curTime - lastNotificationTime_ >= 30 * Constants::MINUTE_MILLIS)
            ) {
                buzzer_->notify();
                lastNotificationTime_ = curTime;
            }
        } else
            warningLevelStartTime_ = MonotonicTime();
    }

    if(changed) {
        comfort_ = comfort;
        ledGroup_->setLed(int(comfort));
    }
}


#if CO2_PWM_SENSOR_ENABLE_PROFILING
    struct Co2PwmSensorProfilingData {
        TimeMicros highLevelDuration;
        TimeMicros lowLevelDuration;
    };

    Co2PwmSensorProfilingData CO2_PWM_SENSOR_PROFILING_DATA;
#endif

enum class Co2PwmSensor::Status: uint8_t {no_data, ok, bounce_error, timing_error, logical_error};
const char* Co2PwmSensor::STATUS_NAMES_[] = {"no-data", "ok", "bounce-error", "timing-error", "logical-error"};

constexpr uint16_t PWM_PIN_UNSET = 0xFFFF;
uint16_t Co2PwmSensor::PWM_PIN_ = PWM_PIN_UNSET;

Co2PwmSensor::State Co2PwmSensor::STATE_ = Co2PwmSensor::State::initializing;
TimeMicros Co2PwmSensor::CYCLE_START_TIME_;
TimeMicros Co2PwmSensor::LOW_LEVEL_START_TIME_;

volatile Co2PwmSensor::Status Co2PwmSensor::STATUS_ = Co2PwmSensor::Status::no_data;
volatile Concentration Co2PwmSensor::CONCENTRATION_;

void Co2PwmSensor::init(uint8_t pwmPin) {
    if(PWM_PIN_ != PWM_PIN_UNSET) {
        UTIL_ASSERT(pwmPin == PWM_PIN_);
        return;
    }

    Util::Core::registerUsedPin(pwmPin);

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
            constexpr TimeMicros highLevelMarkDuration = lround(2000 * Config::CO2_PWM_SENSOR_TIME_SCALE_FACTOR);
            constexpr TimeMicros lowLevelMarkDuration = lround(2000 * Config::CO2_PWM_SENSOR_TIME_SCALE_FACTOR);
            constexpr TimeMicros dataDuration = lround(1000000L * Config::CO2_PWM_SENSOR_TIME_SCALE_FACTOR);
            constexpr TimeMicros cycleDuration = lround(1004000L * Config::CO2_PWM_SENSOR_TIME_SCALE_FACTOR);
            constexpr TimeMicros minPrecision = dataDuration / maxPpm / 2;

            #if CO2_PWM_SENSOR_ENABLE_PROFILING
                CO2_PWM_SENSOR_PROFILING_DATA = Co2PwmSensorProfilingData{
                    highLevelDuration: curHighLevelDuration,
                    lowLevelDuration: curLowLevelDuration,
                };
            #endif

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
#if CO2_PWM_SENSOR_ENABLE_PROFILING
    Co2PwmSensorProfilingData profilingData;
#endif

    noInterrupts();
    *status = STATUS_;
    *concentration = CONCENTRATION_;
#if CO2_PWM_SENSOR_ENABLE_PROFILING
    profilingData = CO2_PWM_SENSOR_PROFILING_DATA;
#endif
    STATUS_ = Status::no_data;
    interrupts();

#if CO2_PWM_SENSOR_ENABLE_PROFILING
    const char* statusName = STATUS_NAMES_[int(*status)];

    if(*status == Status::ok || *status == Status::timing_error) {
        log_debug(
            F("CO2 PWM sensor profiling: "), statusName,
            F(". Logic level durations: "), profilingData.highLevelDuration, F(" + "), profilingData.lowLevelDuration,
            F(" = "), profilingData.highLevelDuration + profilingData.lowLevelDuration, F("."));
    } else {
        log_debug(F("CO2 PWM sensor profiling: "), statusName, F("."));
    }
#endif
}

#if CO2_PWM_SENSOR_ENABLE_PROFILING
void Co2PwmSensor::profile(uint8_t pwmPin) {
    Co2PwmSensor::init(pwmPin);

    Status status;
    Concentration concentration;

    while(true) {
        acquireCurrentStatus(&status, &concentration);
        delay(2000);
    }
}
#endif

Co2PwmSensor::Co2PwmSensor(uint8_t pwmPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup,
                           Display* display, Buzzer* buzzer)
: Co2Sensor(scheduler, ledGroup, display, buzzer) {
    this->init(pwmPin);
}

void Co2PwmSensor::execute() {
    Status status;
    Concentration concentration;
    this->acquireCurrentStatus(&status, &concentration);

    if(status == Status::ok) {
        this->onConcentration(concentration);
    } else {
        log_error(F("Error while reading CO2 concentration using PWM: "), STATUS_NAMES_[int(status)], F("."));
        this->onError();
    }

    this->scheduleAfter(POLLING_PERIOD);
}

const char* Co2PwmSensor::getMode() {
    return "PWM";
}


enum class Co2UartSensor::State: uint8_t {read, reading};
Co2UartSensor::StateHandler Co2UartSensor::stateHandlers_[] = {
    &Co2UartSensor::onReadConcentration,
    &Co2UartSensor::onReadingConcentration,
};

Co2UartSensor::Co2UartSensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup,
                             Display* display, Buzzer* buzzer)
: Co2Sensor(scheduler, ledGroup, display, buzzer), sensorSerial_(sensorSerial), state_(State::read) {
    sensorSerial_->begin(this->SERIAL_SPEED);
}

void Co2UartSensor::execute() {
    size_t handlerId = size_t(state_);
    UTIL_ASSERT(handlerId < UTIL_ARRAY_SIZE(stateHandlers_));
    (this->*stateHandlers_[handlerId])();
}

const char* Co2UartSensor::getMode() {
    return "UART";
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

    // Data is encoded as: Start bit + 8 bits of payload + Stop bit
    constexpr size_t dataSize = (sizeof getGasConcentrationCommand + sizeof response_) * 10;
    constexpr double dataSpeed = double(this->SERIAL_SPEED) / Constants::SECOND_MILLIS;
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
            log_error(F("CO2 sensor has timed out."));
            return this->onCommunicationError();
        }

        return this->scheduleAfter(1);
    }

    byte checksum = 0;
    for(int i = 1; i < responseSize - 1; i++)
        checksum += response_[i];
    checksum = byte(0xFF) - checksum + 1;

    if(response_[0] != 0xFF || response_[responseSize - 1] != checksum || response_[1] != 0x86) {
        log_error(F("CO2 sensor response validation error."));
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
