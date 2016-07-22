// FIXME: Measure current consumption of the scheme. Can Arduino Uno provide it without any problems?

#include <Util.h>
#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>
#include <Util/TaskScheduler.hpp>

#include "Buzzer.hpp"
#include "Co2Sensor.hpp"
#include "Config.hpp"
#include "Dht22.hpp"
#include "Indication.hpp"
#include "TemperatureSensor.hpp"

using Util::Logging::log;

#if CONFIG_ENABLE_TRANSMITTER
    const int TRANSMITTER_SPEED = 1000;
    const int TRANSMITTER_TX_PIN = 9;
    const int TRANSMITTER_RX_PIN = A4;
    const int TRANSMITTER_PTT_PIN = A5;

    // FIXME: Add unusable PWM notes (because of Timer1 usage)
    #include <RH_ASK.h>
    #include "Transmitter.hpp"
    RH_ASK TRANSMITTER(TRANSMITTER_SPEED, TRANSMITTER_RX_PIN, TRANSMITTER_TX_PIN, TRANSMITTER_PTT_PIN);
#endif

#if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
    // AltSoftSerial always uses these pins and breaks PWM on the following pins:
    //
    //           Board          TX  RX  Unusable PWM
    // Arduino Uno, Mini         9   8            10
    // Arduino Leonardo, Micro   5  13        (none)
    // Arduino Mega             46  48        44, 45
    #include <AltSoftSerial.h>
    AltSoftSerial SOFTWARE_SERIAL;
#endif

// MH-Z19 connection notes:
// Vin - 5V
// RX/TX - should be connected to Arduino pins through 5V -> 3.3V logic level shifter
const int CO2_SENSOR_RX_PIN = 9;
const int CO2_SENSOR_TX_PIN = 8;

// DHT22 connection notes:
// One 100nF capacitor should be added between VDD and GND for wave filtering.
const int DHT_22_SENSOR_PIN = 12;

const int SHIFT_REGISTER_DATA_PIN = 10; // SER
const int SHIFT_REGISTER_CLOCK_PIN = 3; // SRCLK
const int SHIFT_REGISTER_LATCH_PIN = 2; // RCLK

const int LIGHT_SENSOR_PIN = A0;
const uint8_t LED_BRIGHTNESS_CONTROLLING_PINS[] = {6};

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int BUZZER_PIN = 11;

void setup() {
    Util::Logging::init();
    // FIXME: check F() macro issues: deduplication, release mode.
    log(F("Initializing..."));

    Util::TaskScheduler scheduler;

    Buzzer buzzer(&scheduler, BUZZER_PIN);
    ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

    const size_t ledsNum = sizeof LED_BRIGHTNESS_CONTROLLING_PINS / sizeof *LED_BRIGHTNESS_CONTROLLING_PINS;
    LedBrightnessRegulator<ledsNum> ledBrightnessRegulator(
        LIGHT_SENSOR_PIN, LED_BRIGHTNESS_CONTROLLING_PINS, &scheduler);

    LedGroup temperatureLeds(&leds, 0, 4);
    LedGroup humidityLeds(&leds, 4, 4);
    Dht22 dht22(DHT_22_SENSOR_PIN, &scheduler, &temperatureLeds, &humidityLeds, &buzzer);

    #if CONFIG_ENABLE_CO2_SENSOR
        LedGroup co2Leds(&leds, 8, 4);

        #if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
            Co2Sensor::SensorSerial* co2SensorSerial = &SOFTWARE_SERIAL;
        #else
            Co2Sensor::SensorSerial* co2SensorSerial = &Serial;
        #endif

        Co2Sensor co2Sensor(co2SensorSerial, &scheduler, &co2Leds, &buzzer);
    #endif

    #if CONFIG_ENABLE_TRANSMITTER
        Transmitter transmitter(&TRANSMITTER, &scheduler, &dht22,
        #if CONFIG_ENABLE_CO2_SENSOR
            &co2Sensor
        #endif
        );
    #endif

    size_t freeMemorySize = getStackFreeMemorySize();
    UTIL_ASSERT(freeMemorySize > 100, F("Failed to start: Not enough memory."));
    log(F("Free memory size: "), freeMemorySize, F(" bytes. Starting the device..."));

    scheduler.run();
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
