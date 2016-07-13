// FIXME: A shitty-written work in progress mockup of communication with DHT22

#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Dht22.hpp"

using Util::Logging::log;
namespace Constants = Util::Constants;

enum class Dht22::State: uint8_t {start_reading, reading};

typedef Dht22::HumidityComfort HumidityComfort;
enum class Dht22::HumidityComfort: uint8_t {unknown, high, normal, low, very_low};
static const char* HUMIDITY_COMFORT_NAMES[] = {"unknown", "high", "normal", "low", "very-low"};

typedef Dht22::TemperatureComfort TemperatureComfort;
enum class Dht22::TemperatureComfort: uint8_t {unknown, cold, normal, warm, hot};
static const char* TEMPERATURE_COMFORT_NAMES[] = {"unknown", "cold", "normal", "warm", "hot"};

namespace {
    constexpr auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
    constexpr auto COLLECTION_PERIOD = 2 * Constants::SECOND_MILLIS;

    // FIXME
    float getTemperature(uint16_t pwmValue) {
        float volts = float(pwmValue) / Constants::ANALOG_HIGH * Constants::VOLTS;
        return (volts - 0.5) * 100;
    }

    // FIXME: choose the right numbers
    // low > 30
    // very low < 30
    // 45% (40-60) - norm
    // high > 60
    //

    // FIXME
    TemperatureComfort getTemperatureComfort(float temperature) {
        long roundedTemperature = lround(temperature);

        if(roundedTemperature <= 18)
            return TemperatureComfort::cold;
        else if(roundedTemperature <= 24)
            return TemperatureComfort::normal;
        else if(roundedTemperature <= 26)
            return TemperatureComfort::warm;
        else
            return TemperatureComfort::hot;
    }
}

Dht22::Dht22(uint8_t dataPin, Util::TaskScheduler* scheduler,
             LedGroup* humidityLedGroup, LedGroup* temperatureLedGroup, Buzzer* buzzer)
: dataPin_(dataPin), state_(State::start_reading), humidityComfort_(HumidityComfort::unknown),
  humidityLedGroup_(humidityLedGroup), humidityLedProgress_(humidityLedGroup),
  temperatureComfort_(TemperatureComfort::unknown), temperatureLedGroup_(temperatureLedGroup),
  temperatureLedProgress_(temperatureLedGroup), buzzer_(buzzer) {
    pinMode(dataPin_, OUTPUT);
    digitalWrite(dataPin_, HIGH);

    scheduler->addTask(&humidityLedProgress_);
    scheduler->addTask(&temperatureLedProgress_);

    scheduler->addTask(this);
    this->scheduleAfter(COLLECTION_PERIOD);
}

void Dht22::execute() {
    switch(state_) {
        case State::start_reading:
            this->onStartReading();
            break;

        case State::reading:
            this->onReading();
            break;

        default:
            UTIL_ASSERT(false);
            break;
    }
}

void Dht22::onStartReading() {
    digitalWrite(dataPin_, LOW);
    this->state_ = State::reading;
    this->scheduleAfter(10);
}

// FIXME: A shitty-written mockup of DHT22 sensor reading
void Dht22::onReading() {
    // FIXME: drop
    TimeMicros startTime = micros();

    digitalWrite(dataPin_, HIGH);
    delayMicroseconds(30);

    pinMode(dataPin_, INPUT);

    if(digitalRead(dataPin_) != LOW) {
        log(F("Failed to receive response from DHT22 on our 'start reading' signal."));
        return this->onError();
    }

    // FIXME: increase timeouts by overhead value?

    // FIXME: drop
    TimeMicros lowVoltageStartTime = micros();

    if(!this->waitForLogicLevel(HIGH, 80)) {
        log(F("Failed to receive 'prepare to receive data' signal from DHT22"));
        return this->onError();
    }

    // FIXME: drop
    TimeMicros highVoltageStartTime = micros();

    if(!this->waitForLogicLevel(LOW, 80)) {
        log(F("Failed to receive 'start data transmission' signal from DHT22"));
        return this->onError();
    }

    // FIXME: drop
    TimeMicros transmissionStartTime = micros();
    log(F("Low level duration: "), highVoltageStartTime - lowVoltageStartTime);
    log(F("High level duration: "), transmissionStartTime - highVoltageStartTime);

    /*
    while(true) {
        // FIXME: negative temperature support
        // uint16_t garbage = receiveData(dataPin_, 1);
        uint16_t first = receiveData(dataPin_, 16);
        uint16_t second = receiveData(dataPin_, 16);
        uint16_t checksum = receiveData(dataPin_, 8);

        uint8_t* data1 = reinterpret_cast<uint8_t*>(&first);
        uint8_t* data2 = reinterpret_cast<uint8_t*>(&second);
        uint8_t dataChecksum = *data1 + *(data1 + 1) + *data2 + *(data2 + 1);

        // interrupts();

        log("Result: ", first, " ", second, " ", checksum == dataChecksum);
        pinMode(dataPin_, OUTPUT);
        digitalWrite(dataPin_, HIGH);
        delay(3000);

    */
    /*
    values_.add(value);

    if(values_.full()) {
        float temperature = getTemperature(value);
        float smoothedTemperature = getTemperature(values_.median());
        this->onTemperature(temperature, smoothedTemperature);
    }
    */
}

// FIXME
void Dht22::onError() {
    UTIL_ASSERT(false);
}

// FIXME: A shitty-written mockup of DHT22 sensor reading
bool Dht22::receiveData(uint16_t* data, uint8_t size) {
    *data = 0;

    // FIXME: drop
    int bitId = 0;
    TimeMicros transmitDurations[size];
    TimeMicros durations[size];

    while(size) {
        // FIXME: increase timeouts by overhead value?

        // FIXME: drop
        TimeMicros transmissionSignalStartTime = micros();

        if(!this->waitForLogicLevel(HIGH, 50)) {
            log(F("Failed to receive 'bit data' signal from DHT22."));
            this->onError();
            return false;
        }

        TimeMicros bitStartTime = micros();

        if(!this->waitForLogicLevel(LOW, 70)) {
            log(F("Failed to receive 'end of bit data' signal from DHT22."));
            this->onError();
            return false;
        }

        TimeMicros bitDuration = micros() - bitStartTime;

        // FIXME: drop
        durations[bitId] = bitDuration;
        transmitDurations[bitId] = bitStartTime - transmissionSignalStartTime;

        *data <<= 1;
        if(bitDuration > 40) // FIXME: alter value
            *data |= 1;

        ++bitId;
    }

    // FIXME: drop
    {
        char buf[100];
        size_t curPos = 0;
        for(int bitId = 0; bitId < size; bitId++) {
            size_t freeSpace = sizeof buf - curPos;
            size_t length = snprintf(buf + curPos, freeSpace, " %ld", durations[bitId]);
            UTIL_ASSERT(length < freeSpace);
            curPos += length;
        }
        // FIXME: check also transmitDurations
        log("Bit durations:", buf);
    }

    return true;
}

bool Dht22::waitForLogicLevel(bool level, TimeMicros timeout) {
    TimeMicros timeoutTime = micros() + timeout;

    do {
        if(digitalRead(dataPin_) == level)
            return true;
    } while(micros() < timeoutTime);

    return false;
}

// FIXME
void Dht22::onTemperature(float temperature, float smoothedTemperature) {
    TemperatureComfort comfort = getTemperatureComfort(smoothedTemperature);

    if(comfort != temperatureComfort_)
        this->onComfortChange(comfort, temperatureComfort_ == TemperatureComfort::unknown);

    log(F("Temperature: "), temperature, F(" -> "), smoothedTemperature,
        F(" ("), TEMPERATURE_COMFORT_NAMES[int(comfort)], F(")."));
}

// FIXME
void Dht22::onComfortChange(TemperatureComfort comfort, bool initialChange) {
    /*
    temperatureComfort_ = comfort;
    ledGroup_->setLed(int(comfort));

    if(initialChange)
        ledProgress_.remove();
    else
        buzzer_->notify();
    */
}
