#include "Config.hpp"

#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "TemperatureSensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

typedef TemperatureSensor::Comfort Comfort;
enum class TemperatureSensor::Comfort: uint8_t {UNKNOWN, COLD, NORMAL, WARM, HOT};
namespace {const char* COMFORT_NAMES[] = {"unknown", "cold", "normal", "warm", "hot"};}

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

TemperatureSensor::TemperatureSensor(uint8_t sensorPin, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: sensorPin_(sensorPin), comfort_(Comfort::UNKNOWN), ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
}

void TemperatureSensor::execute() {
    uint16_t value = analogRead(sensorPin_);
    values_.add(value);

    if(values_.full()) {
        float temperature = getTemperature(value);
        float smoothedTemperature = getTemperature(values_.median());
        this->onTemperature(temperature, smoothedTemperature);
    }

    this->scheduleAfter(500);
}

void TemperatureSensor::onTemperature(float temperature, float smoothedTemperature) {
    Comfort comfort = getComfort(smoothedTemperature);

    if(comfort != comfort_)
        this->onComfortChange(comfort, comfort_ == Comfort::UNKNOWN);

    log("Temperature: ", temperature, " -> ", smoothedTemperature,
        " (", COMFORT_NAMES[int(comfort)], ")");
}

void TemperatureSensor::onComfortChange(Comfort comfort, bool initialChange) {
    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));

    if(initialChange)
        ledProgress_.remove();
    else
        buzzer_->notify();
}
