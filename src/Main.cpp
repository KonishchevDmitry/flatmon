#include <AltSoftSerial.h>

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "CO2Sensor.hpp"
#include "Esp8266.hpp"
#include "Indication.hpp"
#include "TemperatureSensor.hpp"

using Util::Logging::log;

// AltSoftSerial always uses these pins and breaks PWM on the following pins:
//
//           Board          TX  RX  Unusable PWM
// Arduino Uno, Mini         9   8            10
// Arduino Leonardo, Micro   5  13        (none)
// Arduino Mega             46  48        44, 45
AltSoftSerial SOFTWARE_SERIAL;

// ESP8266 connection notes:
//
// VCC - 3.3V, but we shouldn't connect it to Arduino's 3.3V pin because ESP8266 can consume up to 200mA and Arduino Uno
//       is not able to provide such current from 3.3V. So we should use AMS1117 5V -> 3.3V step down power supply module.
// RST, EN (CH_PD), GPIO0, GPIO2, TXD0 - 3.3V through 10K resistor
// GPIO15 - GND through 10K resistor
// TXD0/RXD0 - should be connected to Arduino pins through 5V -> 3.3V logic level shifter
//
// In order to prevent resets the following capacitors should be included to the scheme:
// * A large capacitor (470uF) across the VCC to GND rails.
// * A 0.1uF decoupling capacitor across the ESP8266 VCC to GND inputs very close to the pins (within 1.5 cm).

// MH-Z19 connection notes:
// Vin - 5V
// RX/TX - should be connected to Arduino pins through 5V -> 3.3V logic level shifter
const int CO2_SENSOR_RX_PIN = 9;
const int CO2_SENSOR_TX_PIN = 8;

// DHT22 connection notes:
// One 100nF capacitor should be added between VDD and GND for wave filtering.
const int DHT_22_SENSOR_PIN = 12;

const int TEMPERATURE_SENSOR_PIN = A0;

const int SHIFT_REGISTER_DATA_PIN = 10; // SER
const int SHIFT_REGISTER_CLOCK_PIN = 3; // SRCLK
const int SHIFT_REGISTER_LATCH_PIN = 2; // RCLK

const int LIGHT_SENSOR_PIN = A1;
const uint8_t LED_BRIGHTNESS_CONTROLLING_PINS[] = {6};

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int BUZZER_PIN = 11;

void setup() {
    Util::Logging::init();
    Util::TaskScheduler scheduler;

    // FIXME
    enum class Mode {NORMAL, ESP8266, DHT22};
    Mode mode = Mode::DHT22;

    if(mode == Mode::ESP8266) {
        Esp8266 esp8266(&SOFTWARE_SERIAL, &scheduler);
        esp8266.execute();
    } else {
        Buzzer buzzer(&scheduler, BUZZER_PIN);
        ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

        const size_t ledsNum = sizeof LED_BRIGHTNESS_CONTROLLING_PINS / sizeof *LED_BRIGHTNESS_CONTROLLING_PINS;
        LedBrightnessRegulator<ledsNum> ledBrightnessRegulator(
            LIGHT_SENSOR_PIN, LED_BRIGHTNESS_CONTROLLING_PINS, &scheduler);

        LedGroup co2Leds(&leds, 0, 4);
        CO2Sensor co2Sensor(&SOFTWARE_SERIAL, &scheduler, &co2Leds, &buzzer);

        LedGroup temperatureLeds(&leds, 4, 4);
        TemperatureSensor temperatureSensor(TEMPERATURE_SENSOR_PIN, &scheduler, &temperatureLeds, &buzzer);

        scheduler.run();
    }

    UTIL_ASSERT(false);
}

void loop() {
}
