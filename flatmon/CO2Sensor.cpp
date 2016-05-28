#include "Config.hpp"

#include <Util/Assertion.hpp>
#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "CO2Sensor.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

// It looks like sensor measures CO2 concentration each ~ 6 seconds.
// It shouldn't be polled more often then once in 10 seconds because in that case it returns strange results.
static const auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
static const auto PREHEAT_TIME = DEBUG_MODE ? POLLING_PERIOD : 3 * Constants::MINUTE_MILLIS;

enum class CO2Sensor::State: uint8_t {read, reading};

enum class CO2Sensor::Comfort: uint8_t {unknown, normal, warning, low, critical};
static const char* COMFORT_NAMES[] = {"unknown", "normal", "warning", "low", "critical"};
typedef CO2Sensor::Comfort Comfort;

Comfort getComfort(uint16_t concentration) {
    // CO2 levels explained:
    // * 300 ppm – normal for village outdoor
    // * 500 ppm – normal for city outdoor
    // * 700-1500 ppm – normal for indoor:
    //   * 1000 ppm - decreased attention, initiative and level of perception
    //   * 1500 ppm - fatigue, hard to make decisions / work with information
    //   * 2000 ppm - apathy, headache, chronic fatigue syndrome

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
: sensorSerial_(sensorSerial), state_(State::read), comfort_(Comfort::unknown),
  ledGroup_(ledGroup), ledProgress_(ledGroup), buzzer_(buzzer) {
    sensorSerial_->begin(9600);
    scheduler->addTask(&ledProgress_);
    scheduler->addTask(this);
    this->scheduleAfter(PREHEAT_TIME);
}

void CO2Sensor::execute() {
    switch(state_) {
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
}

void CO2Sensor::onReadConcentration() {
    sensorSerial_->flushInput();
    receivedBytes_ = 0;

    static byte getGasConcentrationCommand[] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
    size_t sentBytes = sensorSerial_->write(getGasConcentrationCommand, sizeof getGasConcentrationCommand);
    UTIL_ASSERT(sentBytes == sizeof getGasConcentrationCommand);

    log("CO2 read start time: ", millis()); // FIXME: drop
    this->state_ = State::reading;
    this->scheduleAfter(0);
}

void CO2Sensor::onReadingConcentration() {
    const int responseSize = sizeof response_;

    while(receivedBytes_ < responseSize) {
        int data = sensorSerial_->read();
        if(data == -1)
            break;

        response_[receivedBytes_++] = data;
    }

    if(receivedBytes_ < responseSize) {
        // FIXME: find out right timeout
        if(this->isTimedOut(5000)) {
            log("CO2 sensor has timed out.");
            this->onCommunicationError();
        }
        return;
    }

    byte checksum = 0;
    for(int i = 1; i < responseSize - 1; i++)
        checksum += response_[i];
    checksum = byte(0xFF) - checksum + 1;

    if(response_[0] != 0xFF || response_[responseSize - 1] != checksum || response_[1] != 0x86) {
        log("CO2 sensor response validation error.");
        this->onCommunicationError();
        return;
    }

    log("CO2 read end time: ", millis()); // FIXME: drop
    uint16_t concentration = uint16_t(response_[2]) << 8 | response_[3];

    Comfort comfort = getComfort(concentration);
    log("CO2: ", concentration, " ppm (", COMFORT_NAMES[int(comfort)], ").");
    this->onComfort(comfort);

    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}

void CO2Sensor::onCommunicationError() {
    this->onComfort(Comfort::unknown);
    this->state_ = State::read;
    this->scheduleAfter(POLLING_PERIOD);
}

void CO2Sensor::onComfort(Comfort comfort) {
    if(comfort == comfort_)
        return;

    if(comfort == Comfort::unknown)
        ledProgress_.resume();
    else if(comfort_ == Comfort::unknown)
        ledProgress_.pause();
    else
        buzzer_->notify();

    comfort_ = comfort;
    ledGroup_->setLed(int(comfort));
}
