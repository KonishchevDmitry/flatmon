#ifndef Indication_tpp
#define Indication_tpp

#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Indication.hpp"

namespace Constants = Util::Constants;

using Util::Logging::log;

template <size_t ledsNum>
LedBrightnessRegulator<ledsNum>::LedBrightnessRegulator(
    uint8_t lightSensorPin, const uint8_t transistorBasePins[ledsNum], Util::TaskScheduler* scheduler
) {
    memcpy(transistorBasePins_, transistorBasePins, sizeof transistorBasePins_);
    #if UTIL_ENABLE_LOGGING
        lastLogTime_ = 0;
    #endif

    pinMode(lightSensorPin_, INPUT);
    for(auto pin : transistorBasePins_)
        pinMode(pin, OUTPUT);

    scheduler->addTask(this);
}

template <size_t ledsNum>
void LedBrightnessRegulator<ledsNum>::execute() {
    uint16_t brightness = Constants::ANALOG_HIGH - analogRead(lightSensorPin_);

    #if UTIL_ENABLE_LOGGING
        auto curTime = millis();
        if(curTime - lastLogTime_ >= 5 * Constants::SECOND_MILLIS) {
            log(F("Brightness: "), brightness, F("."));
            lastLogTime_ = curTime;
        }
    #endif

    // e-Exponential regression calculated by http://keisan.casio.com/exec/system/14059930754231
    // using the following data (determined experimentally):
    // 700 5
    // 800 10
    // 1000 100
    double A = 0.705156848;
    double B = 0.00396581331;
    double x = brightness;
    double y = A * pow(M_E, B * x);

    uint8_t pwmValue = constrain(y, Constants::PWM_LOW + 1, Constants::PWM_HIGH);
    for(auto pin : transistorBasePins_)
        analogWrite(pin, pwmValue);

    this->scheduleAfter(100);
}

#endif
