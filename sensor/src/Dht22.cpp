// FIXME: A shitty-written work in progress mockup of communication with DHT22

#include <Util/Constants.hpp>
#include <Util/Core.hpp>
#include <Util/Logging.hpp>

#include "Dht22.hpp"

#define DHT22_ENABLE_PROFILING 0
#define DHT22_ENABLE_VERBOSE_PROFILING DHT22_ENABLE_PROFILING && UTIL_LOG_VERBOSITY > 0

using Util::Logging::log;
using Util::Logging::vlog;
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

void Dht22::onReading() {
    #if DHT22_ENABLE_PROFILING
        TimeMicros readingStartTime = micros();
    #endif

    digitalWrite(dataPin_, HIGH);
    delayMicroseconds(30);

    pinMode(dataPin_, INPUT);

    if(digitalRead(dataPin_) != LOW) {
        log(F("Failed to receive response from DHT22 on our 'start reading' signal."));
        return this->onError();
    }

    #if DHT22_ENABLE_PROFILING
        TimeMicros lowLevelStartTime = micros();
    #endif

    if(!this->waitForLogicLevel(HIGH, 80)) {
        log(F("Failed to receive 'prepare to receive data' signal from DHT22."));
        return this->onError();
    }

    #if DHT22_ENABLE_PROFILING
        TimeMicros highLevelStartTime = micros();
    #endif

    if(!this->waitForLogicLevel(LOW, 80)) {
        log(F("Failed to receive 'start data transmission' signal from DHT22."));
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

        log(F("DHT22 low level duration: "), highLevelStartTime - lowLevelStartTime, F("."));
        log(F("DHT22 high level duration: "), transmissionStartTime - highLevelStartTime, F("."));
        log(F("DHT22 total reading time: "), readingEndTime - readingStartTime, F("."));
    #endif

    uint8_t payloadChecksum = 0;
    for(size_t byteId = 0; byteId < sizeof payload; byteId++)
        payloadChecksum += reinterpret_cast<uint8_t*>(payload)[byteId];

    if(payloadChecksum != checksum) {
        log(F("Got a corrupted message from DHT22: checksum mismatch."));
        return this->onError();
    }

    int8_t humidity = lround(float(payload[0]) / 10);
    int8_t temperature = lround(float(payload[1]) / 10);
    log(F("Humidity: "), int(humidity), F("%."));
    log(F("Temperature: "), int(temperature), F("C."));

    // FIXME: on error
    pinMode(dataPin_, OUTPUT);
    digitalWrite(dataPin_, HIGH);

    this->state_ = State::start_reading;
    this->scheduleAfter(POLLING_PERIOD);

    // FIXME
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
            log(F("Failed to receive 'bit data' signal from DHT22."));
            return false;
        }

        TimeMicros bitStartTime = micros();

        if(!this->waitForLogicLevel(LOW, 70)) {
            log(F("Failed to receive 'end of bit data' signal from DHT22."));
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
        vlog(F("DHT22 transmission signal durations:"), durationsBuf);

        bufPos = 0;
        for(TimeMicros bitDuration : bitDurations) {
            bufPos += snprintf(durationsBuf + bufPos, sizeof durationsBuf - bufPos, " %ld", bitDuration);
            UTIL_ASSERT(bufPos < sizeof durationsBuf);
        }
        vlog(F("DHT22 bit durations:"), durationsBuf);
    }
    #endif

    return true;
}

bool Dht22::waitForLogicLevel(bool level, TimeMicros timeout) {
    const TimeMicros precision = 10; // FIXME
    TimeMicros timeoutTime = micros() + timeout + precision;

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
