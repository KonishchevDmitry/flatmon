#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "PressureSensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class PressureSensor::State: uint8_t {initialize, start_reading, read_temperature, read_pressure};
PressureSensor::StateHandler PressureSensor::stateHandlers_[] = {
    &PressureSensor::onInitialize,
    &PressureSensor::onStartReading,
    &PressureSensor::onReadTemperature,
    &PressureSensor::onReadPressure,
};

typedef PressureSensor::Comfort Comfort;
enum class PressureSensor::Comfort: uint8_t {unknown, normal, warning, high, critical};
static const char* COMFORT_NAMES[] = {"unknown", "normal", "warning", "high", "critical"};

namespace {
    constexpr auto STARTUP_TIME = 20;
    constexpr auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;

    Comfort getComfort(uint16_t dispersion) {
        if(dispersion < 60)
            return Comfort::normal;
        else if(dispersion < 75)
            return Comfort::warning;
        else if(dispersion < 100)
            return Comfort::high;
        else
            return Comfort::critical;
    }
}

PressureSensor::PressureSensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Display* display)
: state_(State::initialize), pressure_(0), comfort_(Comfort::unknown),
  ledGroup_(ledGroup), ledProgress_(ledGroup), display_(display) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
    this->scheduleAfter(STARTUP_TIME);
}

bool PressureSensor::getPressure(uint16_t* pressure) const {
    if(!pressure_)
        return false;

    *pressure = pressure_;
    return true;
}

void PressureSensor::execute() {
    size_t handlerId = size_t(state_);
    UTIL_ASSERT(handlerId < sizeof stateHandlers_ / sizeof *stateHandlers_);
    (this->*stateHandlers_[handlerId])();
}

void PressureSensor::onInitialize() {
    if(!barometer_.begin())
        return this->onError(F("Failed to initialize the barometer device"));

    curHourStartTime_ = millis();
    pressureHistory_.add({min: 0, max: 0});

    state_ = State::start_reading;
}

void PressureSensor::onStartReading() {
    TimeMillis timeSinceHourStartTime = millis() - curHourStartTime_;

    if(timeSinceHourStartTime >= Constants::HOUR_MILLIS) {
        UTIL_ASSERT(timeSinceHourStartTime < 2 * Constants::HOUR_MILLIS);
        log(F("Close pressure history collection for the current hour."));
        curHourStartTime_ += Constants::HOUR_MILLIS;
        pressureHistory_.add({min: 0, max: 0});
    }

    TimeMillis waitTime = barometer_.startTemperature();
    if(!waitTime)
        return this->onError(F("Failed to begin temperature reading from the barometer"));

    state_ = State::read_temperature;
    this->scheduleAfter(waitTime);
}

void PressureSensor::onReadTemperature() {
    if(!barometer_.getTemperature(temperature_))
        return this->onError(F("Failed to read temperature from the barometer"));

    TimeMillis waitTime = barometer_.startPressure(3);
    if(!waitTime)
        return this->onError(F("Failed to begin pressure reading from the barometer"));

    state_ = State::read_pressure;
    this->scheduleAfter(waitTime);
}

void PressureSensor::onReadPressure() {
    double mbarPressure;
    if(!barometer_.getPressure(mbarPressure, temperature_))
        return this->onError(F("Failed to read pressure from the barometer"));

    pressure_ = lround(mbarPressure * 0.75006375541921 * 10);
    PressureHistory* hourHistory = &pressureHistory_[pressureHistory_.size() - 1];

    uint16_t minPressure;
    if(hourHistory->min && pressure_ >= hourHistory->min)
        minPressure = hourHistory->min;
    else
        minPressure = hourHistory->min = pressure_;

    uint16_t maxPressure;
    if(hourHistory->max && pressure_ <= hourHistory->max)
        maxPressure = hourHistory->max;
    else
        maxPressure = hourHistory->max = pressure_;

    for(size_t hour = 0; hour < pressureHistory_.size() - 1; hour++) {
        PressureHistory* hourHistory = &pressureHistory_[hour];
        if(hourHistory->min && hourHistory->min < minPressure)
            minPressure = hourHistory->min;
        if(hourHistory->max && hourHistory->max > maxPressure)
            maxPressure = hourHistory->max;
    }

    uint16_t dispersion = maxPressure - minPressure;
    Comfort comfort = getComfort(dispersion);

    log(F("Pressure: "), pressure_ / 10, F("."), pressure_ % 10, F("/"), dispersion / 10, F("."), dispersion % 10,
        F(" mmHg ("), COMFORT_NAMES[int(comfort)], F(")."));

    if(display_)
        display_->setPressure(pressure_, dispersion);

    this->onComfort(comfort);

    state_ = State::start_reading;
    this->scheduleAfter(POLLING_PERIOD);
}

void PressureSensor::onError(const FlashChar *error) {
    log(error, F(": I2C error code #"), int(barometer_.getError()), F("."));

    if(display_)
        display_->resetPressure();

    pressure_ = 0;
    this->onComfort(Comfort::unknown);

    if(state_ != State::initialize)
        state_ = State::start_reading;

    this->scheduleAfter(POLLING_PERIOD);
}

void PressureSensor::onComfort(Comfort comfort) {
    if(comfort == comfort_)
        return;

    if(comfort == Comfort::unknown)
        ledProgress_.resume();
    else if(comfort_ == Comfort::unknown)
        ledProgress_.pause();

    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));
}
