#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "TimeSensitiveTasks.hpp"
#include "Dht22.hpp"

#define DHT22_ENABLE_PROFILING 0
#define DHT22_ENABLE_VERBOSE_PROFILING 0

using Util::Logging::log_error;
using Util::Logging::log_info;
using Util::Logging::log_debug;
namespace Constants = Util::Constants;

enum class Dht22::State: uint8_t {start_reading, reading};
Dht22::StateHandler Dht22::stateHandlers_[] = {
    &Dht22::onStartReading,
    &Dht22::onReading,
};

typedef Dht22::TemperatureComfort TemperatureComfort;
enum class Dht22::TemperatureComfort: uint8_t {unknown, cold, normal, warm, hot};
static const char* TEMPERATURE_COMFORT_NAMES[] = {"unknown", "cold", "normal", "warm", "hot"};

typedef Dht22::HumidityComfort HumidityComfort;
enum class Dht22::HumidityComfort: uint8_t {unknown, high, normal, low, very_low};
static const char* HUMIDITY_COMFORT_NAMES[] = {"unknown", "high", "normal", "low", "very-low"};

namespace {
    const auto POLLING_PERIOD = 10 * Constants::SECOND_MILLIS;
    const auto COLLECTION_PERIOD = 2 * Constants::SECOND_MILLIS;

    TemperatureComfort getTemperatureComfort(int8_t temperature) {
        if(temperature <= 20)
            return TemperatureComfort::cold;
        else if(temperature <= 26)
            return TemperatureComfort::normal;
        else if(temperature <= 28)
            return TemperatureComfort::warm;
        else
            return TemperatureComfort::hot;
    }

    HumidityComfort getHumidityComfort(uint8_t humidity) {
        if(humidity <= 30)
            return HumidityComfort::very_low;
        else if(humidity <= 40)
            return HumidityComfort::low;
        else if(humidity <= 60)
            return HumidityComfort::normal;
        else
            return HumidityComfort::high;
    }
}

Dht22::Dht22(uint8_t dataPin, Util::TaskScheduler* scheduler,
             LedGroup* temperatureLedGroup, LedGroup* humidityLedGroup, Display* display)
: dataPin_(dataPin), state_(State::start_reading), temperatureComfort_(TemperatureComfort::unknown),
  temperatureLedGroup_(temperatureLedGroup), temperatureLedProgress_(temperatureLedGroup),
  humidityComfort_(HumidityComfort::unknown), humidityLedGroup_(humidityLedGroup),
  humidityLedProgress_(humidityLedGroup), display_(display) {
    Util::Core::registerUsedPin(dataPin);
    this->stopReading();

    scheduler->addTask(&temperatureLedProgress_);
    scheduler->addTask(&humidityLedProgress_);

    scheduler->addTask(this);
    this->scheduleAfter(COLLECTION_PERIOD);
}

bool Dht22::getTemperature(int8_t *temperature) const {
    if(temperatureComfort_ == TemperatureComfort::unknown)
        return false;

    *temperature = temperature_;
    return true;
}

bool Dht22::getHumidity(uint8_t *humidity) const {
    if(humidityComfort_ == HumidityComfort::unknown)
        return false;

    *humidity = humidity_;
    return true;
}

void Dht22::execute() {
    size_t handlerId = size_t(state_);
    UTIL_ASSERT(handlerId < UTIL_ARRAY_SIZE(stateHandlers_));
    (this->*stateHandlers_[handlerId])();
}

void Dht22::onStartReading() {
    if(!TimeSensitiveTasks::acquire(this)) {
        if(this->isTimedOut(Constants::SECOND_MILLIS)) {
            log_error(F("DHT22 task has timed out on waiting for a time-sensitive task."));
            this->onError();
        }
        return;
    }

    digitalWrite(dataPin_, LOW);
    this->state_ = State::reading;
    this->scheduleAfter(2);
}

void Dht22::onReading() {
    #if DHT22_ENABLE_PROFILING
        TimeMicros readingStartTime = micros();
    #endif

    digitalWrite(dataPin_, HIGH);
    delayMicroseconds(30);

    pinMode(dataPin_, INPUT);

    if(digitalRead(dataPin_) != LOW) {
        log_error(F("Failed to receive response from DHT22 on our 'start reading' signal."));
        return this->onError();
    }

    #if DHT22_ENABLE_PROFILING
        TimeMicros lowLevelStartTime = micros();
    #endif

    if(!this->waitForLogicLevel(HIGH, 80)) {
        log_error(F("Failed to receive 'prepare to receive data' signal from DHT22."));
        return this->onError();
    }

    #if DHT22_ENABLE_PROFILING
        TimeMicros highLevelStartTime = micros();
    #endif

    if(!this->waitForLogicLevel(LOW, 80)) {
        log_error(F("Failed to receive 'start data transmission' signal from DHT22."));
        return this->onError();
    }

    #if DHT22_ENABLE_PROFILING
        TimeMicros transmissionStartTime = micros();
    #endif

    uint16_t payload[2], checksum;
    if(!receiveData(&payload[0], 16) || !receiveData(&payload[1], 16) || !receiveData(&checksum, 8))
        return this->onError();

    #if DHT22_ENABLE_PROFILING
        TimeMicros readingEndTime = micros();

        log_debug(F("DHT22 low level duration: "), highLevelStartTime - lowLevelStartTime, F("."));
        log_debug(F("DHT22 high level duration: "), transmissionStartTime - highLevelStartTime, F("."));
        log_debug(F("DHT22 total reading time: "), readingEndTime - readingStartTime, F("."));
    #endif

    uint8_t payloadChecksum = 0;
    for(size_t byteId = 0; byteId < sizeof payload; byteId++)
        payloadChecksum += reinterpret_cast<uint8_t*>(payload)[byteId];

    if(payloadChecksum != checksum) {
        log_error(F("Got a corrupted message from DHT22: checksum mismatch."));
        return this->onError();
    }

    humidity_ = lroundf(float(payload[0]) / 10);
    HumidityComfort humidityComfort = getHumidityComfort(humidity_);

    int16_t encodedTemperature = payload[1];
    uint16_t negativeTemperatureBit = 1 << 15;
    if(encodedTemperature & negativeTemperatureBit) {
        encodedTemperature ^= negativeTemperatureBit;
        encodedTemperature = -encodedTemperature;
    }

    temperature_ = lroundf(float(encodedTemperature) / 10);
    TemperatureComfort temperatureComfort = getTemperatureComfort(temperature_);

    log_info(F("Humidity: "), humidity_, F("% ("), HUMIDITY_COMFORT_NAMES[int(humidityComfort)], F(")."));
    log_info(F("Temperature: "), temperature_, F("C ("), TEMPERATURE_COMFORT_NAMES[int(temperatureComfort)], F(")."));

    if(display_) {
        display_->setHumidity(humidity_);
        display_->setTemperature(temperature_);
    }

    this->onHumidityComfort(humidityComfort);
    this->onTemperatureComfort(temperatureComfort);

    this->stopReading();
    this->state_ = State::start_reading;
    this->scheduleAfter(POLLING_PERIOD);
}

void Dht22::onError() {
    this->stopReading();

    if(display_) {
        display_->resetHumidity();
        display_->resetTemperature();
    }

    this->onHumidityComfort(HumidityComfort::unknown);
    this->onTemperatureComfort(TemperatureComfort::unknown);

    this->state_ = State::start_reading;
    this->scheduleAfter(POLLING_PERIOD);
}

void Dht22::onTemperatureComfort(TemperatureComfort comfort) {
    if(comfort == temperatureComfort_)
        return;

    if(comfort == TemperatureComfort::unknown)
        temperatureLedProgress_.resume();
    else if(temperatureComfort_ == TemperatureComfort::unknown)
        temperatureLedProgress_.pause();

    temperatureComfort_ = comfort;
    temperatureLedGroup_->setLed(int(comfort));

}

void Dht22::onHumidityComfort(HumidityComfort comfort) {
    if(comfort == humidityComfort_)
        return;

    if(comfort == HumidityComfort::unknown)
        humidityLedProgress_.resume();
    else if(humidityComfort_ == HumidityComfort::unknown)
        humidityLedProgress_.pause();

    humidityComfort_ = comfort;
    humidityLedGroup_->setLed(int(comfort));
}

bool Dht22::receiveData(uint16_t* data, uint8_t size) {
    *data = 0;

    constexpr TimeMicros zeroBitDuration = 28;
    constexpr TimeMicros oneBitDuration = 70;
    constexpr TimeMicros oneBitStartDuration = (oneBitDuration - zeroBitDuration) / 2 + zeroBitDuration;

    #if DHT22_ENABLE_VERBOSE_PROFILING
        uint8_t bitsNum = size;
        TimeMicros transmissionSignalDurations[bitsNum];
        TimeMicros bitDurations[bitsNum];
    #endif

    while(size) {
        #if DHT22_ENABLE_VERBOSE_PROFILING
            TimeMicros transmissionStartTime = micros();
        #endif

        if(!this->waitForLogicLevel(HIGH, 50)) {
            log_error(F("Failed to receive 'bit data' signal from DHT22."));
            return false;
        }

        TimeMicros bitStartTime = micros();

        if(!this->waitForLogicLevel(LOW, 70)) {
            log_error(F("Failed to receive 'end of bit data' signal from DHT22."));
            return false;
        }

        TimeMicros bitDuration = micros() - bitStartTime;

        #if DHT22_ENABLE_VERBOSE_PROFILING
            bitDurations[bitsNum - size] = bitDuration;
            transmissionSignalDurations[bitsNum - size] = bitStartTime - transmissionStartTime;
        #endif

        *data <<= 1;
        if(bitDuration >= oneBitStartDuration)
            *data |= 1;

        --size;
    }

    #if DHT22_ENABLE_VERBOSE_PROFILING
    {
        char durationsBuf[3 * bitsNum + 1];
        size_t bufPos;

        bufPos = 0;
        for(TimeMicros transmissionSignalDuration : transmissionSignalDurations) {
            bufPos += snprintf(durationsBuf + bufPos, sizeof durationsBuf - bufPos, " %ld", transmissionSignalDuration);
            UTIL_ASSERT(bufPos < sizeof durationsBuf);
        }
        log_debug(F("DHT22 transmission signal durations:"), durationsBuf);

        bufPos = 0;
        for(TimeMicros bitDuration : bitDurations) {
            bufPos += snprintf(durationsBuf + bufPos, sizeof durationsBuf - bufPos, " %ld", bitDuration);
            UTIL_ASSERT(bufPos < sizeof durationsBuf);
        }
        log_debug(F("DHT22 bit durations:"), durationsBuf);
    }
    #endif

    return true;
}

bool Dht22::waitForLogicLevel(bool level, TimeMicros timeout) {
    TimeMicros startTime = micros();
    timeout += 15; // DHT22 + measuring precision

    do {
        if(digitalRead(dataPin_) == level)
            return true;
    } while(micros() - startTime < timeout);

    return false;
}

void Dht22::stopReading() {
    pinMode(dataPin_, OUTPUT);
    digitalWrite(dataPin_, HIGH);
    TimeSensitiveTasks::release(this);
}
