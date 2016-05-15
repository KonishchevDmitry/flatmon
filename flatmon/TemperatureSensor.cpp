#include "Config.hpp"

#include <Util/Constants.hpp>
#include <Util/Logging.hpp>

#include "TemperatureSensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;
typedef TemperatureSensor::Comfort Comfort;

namespace {
    const char* COMFORT_NAMES[] = {"unknown", "cold", "normal", "warm", "hot"};
}

float getTemperature(uint16_t pwmValue) {
    float volts = float(pwmValue) / Constants::ANALOG_HIGH * Constants::VOLTS;
    return (volts - 0.5) * 100;
}

Comfort getComfort(float temperature) {
    long roundedTemperature = lround(temperature);

    if(roundedTemperature <= 18)
        return Comfort::COLD;
    else if(roundedTemperature <= 24)
        return Comfort::NORMAL;
    else if(roundedTemperature <= 26)
        return Comfort::WARM;
    else
        return Comfort::HOT;
}

TemperatureSensor::TemperatureSensor(Util::TaskScheduler* scheduler, LedGroup* ledGroup, uint8_t sensorPin)
: ledGroup_(ledGroup), ledProgress_(ledGroup), sensorPin_(sensorPin), comfort_(Comfort::UNKNOWN) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
}

void TemperatureSensor::execute() {
    values_.add(analogRead(sensorPin_));
    if(values_.full())
        this->onTemperature(getTemperature(values_.median()));

    this->scheduleAfter(500);
}

void TemperatureSensor::onTemperature(float temperature) {
    if(comfort_ == Comfort::UNKNOWN)
        ledProgress_.remove();

    Comfort comfort = getComfort(temperature);
    if(comfort_ != comfort) {
        ledGroup_->setLed(int(comfort));
        comfort_ = comfort;
        //notifyComfortChange();
    }

    log("Temperature: ", temperature, " (", COMFORT_NAMES[int(comfort)], ")");
}
