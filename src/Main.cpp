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
// FIXME
// AltSoftSerial SOFTWARE_SERIAL;
#include <SoftwareSerial.h>
SoftwareSerial SOFTWARE_SERIAL(8, 9);

// ESP8266 notes (see http://esp8266.ru/esp8266-podkluchenie-obnovlenie-proshivki/):
// VCC - 3.3V, but we shouldn't connect it to Arduino's 3.3V pin because ESP8266 can consume up to 200mA and Arduino Uno
//       is not able to provide such current from 3.3V. So we should use AMS1117 5V -> 3.3V step down power supply module.
// RST, EN (CH_PD), GPIO0, GPIO2 - 3.3V through 10K resistor
// GPIO15 - GND through 10K resistor
// TXD0/RXD0 - should be connected to Arduino pins through 5V -> 3.3V logic level shifter

// MH-Z19 notes:
// Vin - 5V
// RX/TX - should be connected to Arduino pins through 5V -> 3.3V logic level shifter
const int CO2_SENSOR_RX_PIN = 9;
const int CO2_SENSOR_TX_PIN = 8;

const int TEMPERATURE_SENSOR_PIN = A0;

const int SHIFT_REGISTER_DATA_PIN = 10; // SER
const int SHIFT_REGISTER_CLOCK_PIN = 3; // SRCLK
const int SHIFT_REGISTER_LATCH_PIN = 2; // RCLK

// Attention: Use of tone() function interferes with PWM output on pins 3 and 11 (on boards other than the Mega).
const int BUZZER_PIN = 11;

#if 1
void setup() {
  SOFTWARE_SERIAL.begin(9600);
  Serial.begin(9600);
  Serial.println("Setup done");
  delay(1000);
  SOFTWARE_SERIAL.write("AT\r\n");
  // SOFTWARE_SERIAL.write("AT+GMR\r\n");
}

void loop() {
  if(SOFTWARE_SERIAL.available())
    Serial.write(SOFTWARE_SERIAL.read());
  // if(Serial.available())
    // SOFTWARE_SERIAL.write(Serial.read());
}
#else
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
#endif
