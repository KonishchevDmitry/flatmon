#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "PressureSensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class PressureSensor::State: uint8_t {initialize, start_temperature_reading, read_temperature, read_pressure};
PressureSensor::StateHandler PressureSensor::stateHandlers_[] = {
    &PressureSensor::onInitialize,
    &PressureSensor::onStartTemperatureReading,
    &PressureSensor::onReadTemperature,
    &PressureSensor::onReadPressure,
};

enum class PressureSensor::Comfort: uint8_t {unknown, normal};
static const char* COMFORT_NAMES[] = {"unknown", "normal"};

namespace {
    constexpr auto STARTUP_TIME = 20;
    constexpr auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
}

PressureSensor::PressureSensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, Display* display)
: state_(State::initialize), comfort_(Comfort::unknown), ledGroup_(ledGroup), ledProgress_(ledGroup),
  display_(display) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
    this->scheduleAfter(STARTUP_TIME);
}

bool PressureSensor::getPressure(uint16_t* pressure) const {
    // FIXME
    if(comfort_ == Comfort::unknown)
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
    if(barometer_.begin())
        this->state_ = State::start_temperature_reading;
    else
        this->onError(F("Failed to initialize the barometer device"));
}

void PressureSensor::onStartTemperatureReading() {
    TimeMillis waitTime = barometer_.startTemperature();
    if(!waitTime)
        return this->onError(F("Failed to begin temperature reading from the barometer"));

    this->state_ = State::read_temperature;
    this->scheduleAfter(waitTime);
}

void PressureSensor::onReadTemperature() {
    if(!barometer_.getTemperature(temperature_))
        return this->onError(F("Failed to read temperature from the barometer"));

    TimeMillis waitTime = barometer_.startPressure(3);
    if(!waitTime)
        return this->onError(F("Failed to begin pressure reading from the barometer"));

    this->state_ = State::read_pressure;
    this->scheduleAfter(waitTime);
}

void PressureSensor::onReadPressure() {
    double pressure;
    if(!barometer_.getPressure(pressure, temperature_))
        return this->onError(F("Failed to read pressure from the barometer"));

    pressure_ = lround(pressure * 0.75006375541921);
    log(F("Pressure: "), pressure_, F(" mmHg."));

    // FIXME
    // if(display_)
        // display_->setHumidity(humidity_);

    // FIXME
    this->onComfort(Comfort::normal);

    this->state_ = State::start_temperature_reading;
    this->scheduleAfter(POLLING_PERIOD);
}

void PressureSensor::onError(const FlashChar *error) {
    // FIXME
    log(error, F(": I2C error code #"), int(barometer_.getError()), F("."));

    // FIXME
    // if(display_) {
    //     display_->resetHumidity();
    //     display_->resetTemperature();
    // }

    this->onComfort(Comfort::unknown);

    if(this->state_ != State::initialize)
        this->state_ = State::start_temperature_reading;

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
