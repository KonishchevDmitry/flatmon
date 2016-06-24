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
        log("Failed to receive start reading response signal from DHT22.");
        this->onError();
    }


    // this->waitForLogicLevel(LOW, 0)

    /*
    while(true) {
        TimeMicros lowVoltageStartTime = micros();

        // Wait for high voltage
        while(!digitalRead(dataPin_))
            ;
        TimeMicros highVoltageStartTime = micros();

        //log("Got high voltage!");

        // Wait transmission start
        while(digitalRead(dataPin_))
            ;

        TimeMicros transmissionStartTime = micros();

        TimeMicros lowVoltageDuration = highVoltageStartTime - lowVoltageStartTime;
        TimeMicros highVoltageDuration = transmissionStartTime - highVoltageStartTime;

        // interrupts();

        if(lowVoltageDuration < 50 || lowVoltageDuration > 90)
            log("Got an invalid low voltage duration: ", lowVoltageDuration);
        if(highVoltageDuration < 60 || highVoltageDuration > 90)
            log("Got an invalid high voltage duration: ", highVoltageDuration);

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
void onError() {
    UTIL_ASSERT(false);
}

bool Dht22::waitForLogicLevel(bool level, TimeMicros timeout) {
    TimeMicros timeoutTime = micros() + timeout;

    do {
        if(digitalRead(dataPin_) == level)
            return true;
    } while(micros() < timeoutTime);

    return false;
}

// FIXME: A shitty-written mockup of DHT22 sensor reading
uint16_t receiveData(uint8_t dataPin_, uint8_t size) {
    uint16_t data = 0;
    TimeMicros transmitDurations[size];
    TimeMicros durations[size];

    for(int bitId = 0; bitId < size; bitId++) {
        // noInterrupts();

        TimeMicros transmissionSignalStartTime = micros();

        // Wait for bit data
        while(!digitalRead(dataPin_))
            ;

        TimeMicros bitStartTime = micros();
        //log("Got start of a bit.");

        while(digitalRead(dataPin_))
            ;

        TimeMicros bitDuration = micros() - bitStartTime;
        TimeMicros transmissionSignalDuration = bitStartTime - transmissionSignalStartTime;

        // interrupts();

        /*
        if(transmissionSignalDuration < 40 || transmissionSignalDuration > 60)
            log("Got an invalid transmission signal duration: ", transmissionSignalDuration);

        if(bitDuration < 20 || bitDuration > 30 && bitDuration < 70 || bitDuration > 80)
            log("Got an invalid bit duration: ", bitDuration);
        */

        durations[bitId] = bitDuration;
        if(bitDuration > 40)
            data |= (uint16_t(1) << (size - bitId - 1));
        // log("Bit value: ", bitDuration);
    }

    // for(int bitId = 0; bitId < size; bitId++)
        // log("Duration: ", transmitDurations[bitId], " ", durations[bitId]);

    return data;
}

// FIXME
void Dht22::onTemperature(float temperature, float smoothedTemperature) {
    TemperatureComfort comfort = getTemperatureComfort(smoothedTemperature);

    if(comfort != temperatureComfort_)
        this->onComfortChange(comfort, temperatureComfort_ == TemperatureComfort::unknown);

    log("Temperature: ", temperature, " -> ", smoothedTemperature,
        " (", TEMPERATURE_COMFORT_NAMES[int(comfort)], ").");
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
