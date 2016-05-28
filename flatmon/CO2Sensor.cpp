#include "Config.hpp"

#include <Util/Assertion.hpp>
#include <Util/Core.hpp>
#include <Util/Constants.hpp>
#include <Util/Logging.hpp>

#include "CO2Sensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

// FIXME: voltage divider for RX
// FIXME: Preheat time - 3 min
// FIXME: Reponse Time - T90 < 60s
// FIXME: min period - 10 seconds. Test?
// FIXME: outdoor calibration
// FIXME: sensor measures CO2 concentration ~ each 6 seconds
// FIXME: autocalibrates during a few days

enum class CO2Sensor::State: uint8_t {
    initializing,
    read,
    reading,
};

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
: sensorSerial_(sensorSerial), state_(State::initializing), comfort_(Comfort::unknown),
  ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    sensorSerial_->begin(9600);
    // FIXME
    //scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
}

void CO2Sensor::execute() {
    switch(state_) {
        case State::initializing:
            state_ = State::read;
            break;
        case State::read:
            this->onReadConcentration();
            break;
        case State::reading:
            this->onReadingConcentration();
            break;
        default:
            UTIL_ASSERT(false);
            break;
    }

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

void CO2Sensor::onReadConcentration() {
    static byte command[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
    sensorSerial_->flushInput();
    UTIL_ASSERT(sensorSerial_->write(command, sizeof command) == sizeof command);
    receivedBytes_ = 0;

    this->state_ = State::reading;
    this->scheduleAfter(0);
}

void CO2Sensor::onReadingConcentration() {
    while(receivedBytes_ < sizeof response_) {
        int data = sensorSerial_->read();
        if(data == -1)
            break;

        response_[receivedBytes_++] = data;
    }

    if(receivedBytes_ < sizeof response_) {
        // FIXME: measure
        if(this->isTimedOut(10)) {
            // FIXME
            log("CO2 sensor has timed out.");
            UTIL_ASSERT(false);
        }

        return;
    }

    log("CO2 sensor data has been read.");
}
#if 0
unsigned char response[9];

void loop() {
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
