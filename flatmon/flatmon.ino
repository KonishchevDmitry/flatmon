#include "Config.hpp"

#include <Util.h>
#include <Util/Logging.hpp>
#include <Util/TaskScheduler.hpp>

#include "Indication.hpp"
#include "TemperatureSensor.hpp"

const int DATA_PIN = 10;
const int CLOCK_PIN = 8;
const int LATCH_PIN = 9;

const int TEMPERATURE_SENSOR_PIN = 0;

void setup() {
    Util::Logging::init();

    //pinMode(SPEAKER_PIN, OUTPUT);

    Util::TaskScheduler scheduler;
    ShiftRegisterLeds leds(DATA_PIN, CLOCK_PIN, LATCH_PIN);

    LedGroup temperatureLeds(&leds, 4, 4);
    TemperatureSensor temperatureSensor(&scheduler, &temperatureLeds, TEMPERATURE_SENSOR_PIN);

    scheduler.run();
}

void loop() {
}

/*
#include <math.h>

#include <Util.h>
#include <Util/Constants.hpp>
#include <Util/CycleBuffer.hpp>

namespace Constants = Util::Constants;



// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int SPEAKER_PIN = 11;



void notifyComfortChange() {
    tone(SPEAKER_PIN, 35);
    delay(100);
    tone(SPEAKER_PIN, 41, 50);
}
*/
