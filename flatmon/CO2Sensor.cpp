#include "Config.hpp"

#include <Util/Core.hpp>
#include <Util/Constants.hpp>
#include <Util/Logging.hpp>

#include "CO2Sensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

// FIXME: read specs
// FIXME: Articles: https://geektimes.ru/post/272090/
// FIXME: voltage divider for RX
// FIXME: 3 min delay
// FIXME: min period - 10 seconds. Test?
// FIXME: outdoor calibration
// FIXME: sensor measures CO2 concentration ~ each 6 seconds
// FIXME: autocalibrates during a few days

typedef CO2Sensor::Comfort Comfort;
enum class CO2Sensor::Comfort: uint8_t {unknown, normal, warning, low, critical};
namespace {const char* COMFORT_NAMES[] = {"unknown", "normal", "warning", "low", "critical"};}

Comfort getComfort(uint16_t concentration) {
    // CO2 levels explained:
    // * 300 ppm – normal for village outdoor
    // * 500 ppm – normal for city outdoor
    // * 700-1500 ppm – normal for indoor:
    //   * 1000 ppm - decreased attention, initiative and level of perception
    //   * 1500 ppm - fatigue, hard to make decisions / work with information
    //   * 2000 ppm - brains are off, apathy, headache, chronic fatigue syndrome

    uint16_t roundedConcentration = lround(float(concentration) / 100) * 100;

    if(roundedConcentration < 900)
        return Comfort::normal;
    else if(roundedConcentration < 1400)
        return Comfort::warning;
    else if(roundedConcentration < 1900)
        return Comfort::low;
    else
        return Comfort::critical;
}

CO2Sensor::CO2Sensor(SensorSerial* sensorSerial, Util::TaskScheduler* scheduler, LedGroup* ledGroup, Buzzer* buzzer)
: sensorSerial_(sensorSerial), comfort_(Comfort::unknown), ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    sensorSerial_->begin(9600);
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
}

#if 0
void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor to open
  Serial.println("AltSoftSerial Test Begin");
  altSerial.begin(9600);
  altSerial.println("Hello World");
}

void loop() {
  char c;

  if (Serial.available()) {
    c = Serial.read();
    altSerial.print(c);
  }
  if (altSerial.available()) {
    c = altSerial.read();
    Serial.print(c);
  }
}
#endif

void CO2Sensor::execute() {
    /*
    uint16_t value = analogRead(sensorPin_);
    values_.add(value);

    // FIXME: Do we need it?
    if(values_.full()) {
        float temperature = getTemperature(value);
        float smoothedTemperature = getTemperature(values_.median());
        this->onTemperature(temperature, smoothedTemperature);
    }

    this->scheduleAfter(500);
    */
}

/*
void CO2Sensor::onTemperature(float temperature, float smoothedTemperature) {
    Comfort comfort = getComfort(smoothedTemperature);

    if(comfort != comfort_)
        this->onComfortChange(comfort, comfort_ == Comfort::UNKNOWN);

    log("Temperature: ", temperature, " -> ", smoothedTemperature,
        " (", COMFORT_NAMES[int(comfort)], ")");
}

void CO2Sensor::onComfortChange(Comfort comfort, bool initialChange) {
    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));

    if(initialChange)
        ledProgress_.remove();
    else
        buzzer_->notify();
}
*/

#if 0
byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
unsigned char response[9];

void loop() {
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc+=response[i];
  crc = 255 - crc;
  crc++;

  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256*responseHigh) + responseLow;
    Serial.println(ppm);
  }

  delay(10000);
}
#endif
