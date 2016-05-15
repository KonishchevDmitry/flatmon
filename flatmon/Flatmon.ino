#include "Config.hpp"

#include <Util.h>
#include <Util/Logging.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Indication.hpp"
#include "TemperatureSensor.hpp"

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int BUZZER_PIN = 11;

const int TEMPERATURE_SENSOR_PIN = 0;

const int SHIFT_REGISTER_DATA_PIN = 10;
const int SHIFT_REGISTER_CLOCK_PIN = 8;
const int SHIFT_REGISTER_LATCH_PIN = 9;

void setup() {
    Util::Logging::init();
    Util::TaskScheduler scheduler;

    Buzzer buzzer(&scheduler, BUZZER_PIN);
    ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

    LedGroup temperatureLeds(&leds, 4, 4);
    TemperatureSensor temperatureSensor(TEMPERATURE_SENSOR_PIN, &scheduler, &temperatureLeds, &buzzer);

    scheduler.run();
}

void loop() {
}
