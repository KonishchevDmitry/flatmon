#include <avr/wdt.h>

#include <EEPROM.h>
#include <Wire.h>

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
#include "Display.hpp"
#include "Indication.hpp"
#include "PressureSensor.hpp"

using Util::Logging::log;

// DHT22 connection notes:
// One 100nF capacitor should be added between VDD and GND for wave filtering.
const int DHT_22_SENSOR_PIN = 12;

// MH-Z19 connection notes:
// Vin - 5V
// HD - 3.3V
// RX/TX should be connected to Arduino pins through 5V -> 3.3V logic level shifter
#if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
    // AltSoftSerial always uses these pins and breaks PWM on the following pins because of timer usage:
    //
    //    Board      TX  RX   Timer  Unusable PWM
    // Arduino Uno    9   8  Timer1            10
    // Arduino Mega  46  48  Timer5        44, 45
    #include <AltSoftSerial.h>
    AltSoftSerial SOFTWARE_SERIAL;

    const int CO2_SENSOR_RX_PIN = 9;
    const int CO2_SENSOR_TX_PIN = 8;
#else
    const int CO2_SENSOR_PWM_PIN = 2;
#endif

// BMP180 connection notes:
// VIN: 3.3V
// GND: GND
// SDA: Uno - A4, Mega - 20
// SCL: Uno - A5, Mega - 21

const int LIGHT_SENSOR_PIN = A0;
const uint8_t LED_BRIGHTNESS_CONTROLLING_PINS[] = {5};

const int SHIFT_REGISTER_DATA_PIN = 6; // SER
const int SHIFT_REGISTER_CLOCK_PIN = 4; // SRCLK
const int SHIFT_REGISTER_LATCH_PIN = 3; // RCLK

// Use of tone() function breaks PWM on the following pins because of Timer2 usage:
//
//    Board      Unusable PWM
// Arduino Uno          3, 11
// Arduino Mega         9, 10
const int BUZZER_PIN = 11;

// LCD connection:
// VSS - GND
// VDD - 5V
// VO (Contrast) - using voltage devider: 5V - 10K - V0 - 1K - GND
const int LCD_RS_PIN = A1;
// RW - GND
const int LCD_E_PIN = A2;
// D0-D3 - not connected
const int LCD_D4_PIN = A3;
const int LCD_D5_PIN = A4;
const int LCD_D6_PIN = A5;
const int LCD_D7_PIN = 10;
// A (LED+ has internal resistor) - 5V
// K (LED-) - GND
Display LCD(LCD_RS_PIN, LCD_E_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

#if CONFIG_ENABLE_TRANSMITTER
    #include <RH_ASK.h>
    #include "Transmitter.hpp"

    // RadioHead breaks PWM on the following pins because of timer usage:
    //
    //    Board       Timer  Unusable PWM
    // Arduino Uno   Timer1         9, 10
    // Arduino Mega  Timer1        11, 12
    const int TRANSMITTER_SPEED = 1000;
    const int TRANSMITTER_TX_PIN = 9;

    RH_ASK_TRANSMITTER TRANSMITTER(TRANSMITTER_SPEED, TRANSMITTER_TX_PIN);
#endif


// EEPROM address where system reset reason is stored:
// 0 - ordinary reset
// 1 - reset by watchdog timer
const uint16_t SYSTEM_RESET_REASON_FLAG_ADDRESS = EEPROM.length() - 1;

class WdtResetter: public Util::Task {
    public:
        WdtResetter(Util::TaskScheduler* scheduler) {
            scheduler->addTask(this);
        }

    public:
        virtual void execute() {
            wdt_reset();
            this->scheduleAfter(10);
        }
};

void abortHandler(
#if UTIL_VERBOSE_ASSERTS
    const FlashChar* file, int line
#endif
) {
    LCD.showAssertionError(
    #if UTIL_VERBOSE_ASSERTS
        file, line
    #endif
    );
}

void initialize() {
    size_t freeMemorySize = Util::Core::getStackFreeMemorySize();
    UTIL_ASSERT(freeMemorySize > 100, F("Failed to start: Not enough memory."));
    log(F("Free memory size: "), freeMemorySize, F(" bytes."));

    if(EEPROM[SYSTEM_RESET_REASON_FLAG_ADDRESS]) {
        EEPROM[SYSTEM_RESET_REASON_FLAG_ADDRESS] = 0;
        log(F("System lockup detected."));
        LCD.showSystemLockupError();
        Util::Core::stopDevice();
    }

    wdt_enable(
    #if UTIL_ENABLE_LOGGING
        WDTO_60MS
    #else
        WDTO_15MS
    #endif
    );
    WDTCSR |= _BV(WDIE);
}

ISR(WDT_vect) {
    EEPROM[SYSTEM_RESET_REASON_FLAG_ADDRESS] = 1;

    // Use this hack to reset the system
    wdt_enable(WDTO_15MS);
    while(true)
        ;
}

void setup() {
    // Disable watchdog timer which may be set if the MCU has been reset by enabled watchdog timer
    wdt_disable();

    Util::Core::init();
    Util::Logging::init();

    log(F("Initializing..."));
    Util::Assertion::setAbortHandler(abortHandler);

    Util::TaskScheduler scheduler;
    WdtResetter wdtResetter(&scheduler);

    Buzzer buzzer(&scheduler, BUZZER_PIN);

    ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);
    const size_t ledsNum = sizeof LED_BRIGHTNESS_CONTROLLING_PINS / sizeof *LED_BRIGHTNESS_CONTROLLING_PINS;
    LedBrightnessRegulator<ledsNum> ledBrightnessRegulator(
        LIGHT_SENSOR_PIN, LED_BRIGHTNESS_CONTROLLING_PINS, &scheduler);

    LedGroup temperatureLeds(&leds, 0, 4);
    LedGroup humidityLeds(&leds, 4, 4);
    Dht22 dht22(DHT_22_SENSOR_PIN, &scheduler, &temperatureLeds, &humidityLeds, &LCD);

    LedGroup co2Leds(&leds, 8, 4);
    #if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
        Co2UartSensor co2Sensor(&SOFTWARE_SERIAL, &scheduler, &co2Leds, &LCD, &buzzer);
    #else
        Co2PwmSensor co2Sensor(CO2_SENSOR_PWM_PIN, &scheduler, &co2Leds, &LCD, &buzzer);
    #endif

    LedGroup pressureLeds(&leds, 12, 4);
    PressureSensor pressureSensor(&scheduler, &pressureLeds, &LCD);

    #if CONFIG_ENABLE_TRANSMITTER
        Transmitter transmitter(&TRANSMITTER, &scheduler, &dht22, &co2Sensor);
    #endif

    initialize();
    log(F("The system has been initialized. Starting the device."));
    scheduler.run();
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
