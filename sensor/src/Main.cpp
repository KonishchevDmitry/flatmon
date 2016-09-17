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

using Util::Logging::log_critical;
using Util::Logging::log_info;
namespace Constants = Util::Constants;

// DHT22 connection notes:
// One 100nF capacitor should be added between VDD and GND for wave filtering.
#if ARDUINO_AVR_MEGA2560
    const int DHT_22_SENSOR_PIN = 29;
#else
    const int DHT_22_SENSOR_PIN = 12;
#endif

// MH-Z19 connection notes:
// Vin - 5V
// HD - 3.3V
// RX/TX should be connected to Arduino pins through 5V -> 3.3V logic level shifter
#if ARDUINO_AVR_MEGA2560
    HardwareSerial* CO2_SENSOR_SERIAL = &Serial3;
    const int CO2_SENSOR_RX_PIN = 15;
    const int CO2_SENSOR_TX_PIN = 14;
#else
    #if CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
        #include <AltSoftSerial.h>
        AltSoftSerial SOFTWARE_SERIAL;
        AltSoftSerial* CO2_SENSOR_SERIAL = &SOFTWARE_SERIAL;

        // AltSoftSerial always uses these pins and breaks PWM on the following pins because of timer usage:
        //
        //    Board      TX  RX   Timer  Unusable PWM
        // Arduino Uno    9   8  Timer1            10
        // Arduino Mega  46  48  Timer5        44, 45
        const int CO2_SENSOR_RX_PIN = 8;
        const int CO2_SENSOR_TX_PIN = 9;
    #else
        const int CO2_SENSOR_PWM_PIN = 2;
    #endif
#endif

// BMP180 connection notes:
//
// VIN: 3.3V
// GND: GND
//
// Via bi-directional logic level converter:
// * SDA: Uno - A4, Mega - 20
// * SCL: Uno - A5, Mega - 21

// 5V - 10K - sensor pin - light-dependent resistor - GND
const int LIGHT_SENSOR_PIN = A0;

#if ARDUINO_AVR_MEGA2560
    const int LCD_BRIGHTNESS_CONTROLLING_PIN = 8;
    const int COMFORT_LEDS_BRIGHTNESS_CONTROLLING_PIN = 6;
#else
    const int LCD_BRIGHTNESS_CONTROLLING_PIN = 5;
#endif

// Shift register connection:
// VCC - 5V
// QA - first light
// SER - for first shift register see pin below, for second shift register - QH' of the first shift register
// OE - GND
// RCLK - see pin below (common for both shift registers)
// SRCLK - see pin below (common for both shift registers)
// SRCLR - 5V
// QH' - SER of next shift register
// QB-QH - leds
// GND - GND
#if ARDUINO_AVR_MEGA2560
    const int SHIFT_REGISTER_DATA_PIN = A3; // SER
    const int SHIFT_REGISTER_LATCH_PIN = A2; // RCLK
    const int SHIFT_REGISTER_CLOCK_PIN = A1; // SRCLK
#else
    const int SHIFT_REGISTER_DATA_PIN = 6; // SER
    const int SHIFT_REGISTER_LATCH_PIN = 3; // RCLK
    const int SHIFT_REGISTER_CLOCK_PIN = 4; // SRCLK
#endif

// Use of tone() function breaks PWM on the following pins because of Timer2 usage:
//
//    Board      Unusable PWM
// Arduino Uno          3, 11
// Arduino Mega         9, 10
#if ARDUINO_AVR_MEGA2560
    const int BUZZER_PIN = 10;
#else
    const int BUZZER_PIN = 11;
#endif

// LCD connection:
// VSS - GND
// VDD - 5V
// VO (Contrast) - using voltage devider: 5V - 10K - V0 - 1K - GND
#if ARDUINO_AVR_MEGA2560
    const int LCD_RS_PIN = 27;
    // RW - GND
    const int LCD_E_PIN = 26;
    // D0-D3 - not connected
    const int LCD_D4_PIN = 25;
    const int LCD_D5_PIN = 24;
    const int LCD_D6_PIN = 23;
    const int LCD_D7_PIN = 22;
#else
    const int LCD_RS_PIN = A1;
    // RW - GND
    const int LCD_E_PIN = A2;
    // D0-D3 - not connected
    const int LCD_D4_PIN = A3;
    const int LCD_D5_PIN = 10;
    const int LCD_D6_PIN = 8;
    const int LCD_D7_PIN = 7;
#endif
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

    #if ARDUINO_AVR_MEGA2560
        const int TRANSMITTER_TX_PIN = 47;
    #else
        const int TRANSMITTER_TX_PIN = 9;
    #endif

    RH_ASK_TRANSMITTER TRANSMITTER(1000, TRANSMITTER_TX_PIN);
#endif


class ComfortLedsBrightnessController: public LedBrightnessController {
    public:
        ComfortLedsBrightnessController(uint8_t transistorBasePin): LedBrightnessController(transistorBasePin) {
        }

    protected:
        virtual uint8_t getPwmValue(uint16_t brightness) {
            // e-Exponential regression calculated by http://keisan.casio.com/exec/system/14059930754231
            // using the following data (determined experimentally):
            // 700 5
            // 800 10
            // 1000 100
            double A = 0.705156848;
            double B = 0.00396581331;
            double x = brightness;
            double y = A * pow(M_E, B * x);
            return constrain(y, Constants::PWM_LOW + 1, Constants::PWM_HIGH);
        };
};

class LcdBrightnessController: public ComfortLedsBrightnessController {
    public:
        LcdBrightnessController(uint8_t transistorBasePin): ComfortLedsBrightnessController(transistorBasePin) {
        }

    protected:
        // FIXME: Generate custom regression for LCD brightness
        virtual uint8_t getPwmValue(uint16_t brightness) {
            uint16_t pwmValue = ComfortLedsBrightnessController::getPwmValue(brightness);
            return constrain(pwmValue + 10, Constants::PWM_LOW + 1, Constants::PWM_HIGH);
        };
};

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
        virtual const FlashChar* getName() {
            return F("WDT resetter");
        }

        virtual void execute() {
            // Arduino's delay() is actually a busy loop with time checking, so it's OK to make busy loop here by not
            // scheduling the task for a later time.
            wdt_reset();
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
    Util::Core::configureUnusedPins();

    size_t freeMemorySize = Util::Core::getStackFreeMemorySize();
    UTIL_ASSERT(freeMemorySize > 100, F("Failed to start: Not enough memory."));
    log_info(F("Free memory size: "), freeMemorySize, F(" bytes."));

    if(EEPROM[SYSTEM_RESET_REASON_FLAG_ADDRESS]) {
        EEPROM[SYSTEM_RESET_REASON_FLAG_ADDRESS] = 0;
        log_critical(F("System lockup detected."));
        LCD.showSystemLockupError();
        Util::Core::stopDevice();
    }

    wdt_enable(
    #if UTIL_LOG_LEVEL == UTIL_LOG_LEVEL_DISABLED
        WDTO_15MS
    #elif UTIL_LOG_LEVEL < UTIL_LOG_LEVEL_INFO
        WDTO_30MS
    #elif UTIL_LOG_LEVEL < UTIL_LOG_LEVEL_DEBUG
        WDTO_60MS
    #else
        WDTO_120MS
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
    Util::Logging::init(115200);

    log_info(F("Initializing..."));
    Util::Assertion::setAbortHandler(abortHandler);

    Util::TaskScheduler scheduler;
    WdtResetter wdtResetter(&scheduler);

    Buzzer buzzer(&scheduler, BUZZER_PIN);

    auto lcdBrightnessController = LcdBrightnessController(LCD_BRIGHTNESS_CONTROLLING_PIN);
#if ARDUINO_AVR_MEGA2560
    auto comfortLedsBrightnessController = ComfortLedsBrightnessController(COMFORT_LEDS_BRIGHTNESS_CONTROLLING_PIN);
#endif
    LedBrightnessController* ledBrightnessControllers[] = {
        &lcdBrightnessController,
    #if ARDUINO_AVR_MEGA2560
        &comfortLedsBrightnessController
    #endif
    };
    LedBrightnessRegulator ledBrightnessRegulator(
        LIGHT_SENSOR_PIN, ledBrightnessControllers, UTIL_ARRAY_SIZE(ledBrightnessControllers), &scheduler);

    ShiftRegisterLeds leds(SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_CLOCK_PIN, SHIFT_REGISTER_LATCH_PIN);

    LedGroup temperatureLeds(&leds, 4, 4);
    LedGroup humidityLeds(&leds, 0, 4);
    Dht22 dht22(DHT_22_SENSOR_PIN, &scheduler, &temperatureLeds, &humidityLeds, &LCD);

    LedGroup co2Leds(&leds, 12, 4);
    #if ARDUINO_AVR_MEGA2560 || CONFIG_CO2_SENSOR_USE_SOFTWARE_SERIAL
        Util::Core::registerUsedPins(CO2_SENSOR_RX_PIN, CO2_SENSOR_TX_PIN);
        Co2UartSensor co2Sensor(CO2_SENSOR_SERIAL, &scheduler, &co2Leds, &LCD, &buzzer);
    #else
        Co2PwmSensor co2Sensor(CO2_SENSOR_PWM_PIN, &scheduler, &co2Leds, &LCD, &buzzer);
    #endif

    LedGroup pressureLeds(&leds, 8, 4);
    PressureSensor pressureSensor(&scheduler, &pressureLeds, &LCD);

    #if CONFIG_ENABLE_TRANSMITTER
        Util::Core::registerUsedPin(TRANSMITTER_TX_PIN);
        // FIXME: Enable transmitter
        // Transmitter transmitter(&TRANSMITTER, &scheduler, &dht22, &co2Sensor, &pressureSensor);
    #endif

    initialize();
    log_info(F("The system has been initialized. Starting the device."));
    scheduler.run();
}

void loop() {
    UTIL_LOGICAL_ERROR();
}
