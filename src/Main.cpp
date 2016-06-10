#include <AltSoftSerial.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "CO2Sensor.hpp"
#include "Indication.hpp"
#include "TemperatureSensor.hpp"

// AltSoftSerial always uses these pins and breaks PWM on the following pins:
//
//           Board          TX  RX  Unusable PWM
// Arduino Uno, Mini         9   8            10
// Arduino Leonardo, Micro   5  13        (none)
// Arduino Mega             46  48        44, 45
AltSoftSerial SOFTWARE_SERIAL;
const int CO2_SENSOR_RX_PIN = 9;
const int CO2_SENSOR_TX_PIN = 8;

const int TEMPERATURE_SENSOR_PIN = A0;

const int SHIFT_REGISTER_DATA_PIN = 10; // SER
const int SHIFT_REGISTER_CLOCK_PIN = 3; // SRCLK
const int SHIFT_REGISTER_LATCH_PIN = 2; // RCLK

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int BUZZER_PIN = 11;

void setup() {
    Util::Logging::init();
    Util::TaskScheduler scheduler;

    Buzzer buzzer(&scheduler, BUZZER_PIN);
    ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

    LedGroup co2Leds(&leds, 0, 4);
    CO2Sensor co2Sensor(&SOFTWARE_SERIAL, &scheduler, &co2Leds, &buzzer);

    LedGroup temperatureLeds(&leds, 4, 4);
    TemperatureSensor temperatureSensor(TEMPERATURE_SENSOR_PIN, &scheduler, &temperatureLeds, &buzzer);

    scheduler.run();

    UTIL_ASSERT(false);
}

void loop() {
}
